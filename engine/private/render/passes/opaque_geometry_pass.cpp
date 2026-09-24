#include "render/passes/opaque_geometry_pass.h"

#include "core/engine.h"
#include "graphics/buffer.h"
#include "graphics/shader.h"
#include "render/frame_params.h"
#include "render/material.h"
#include "render/mesh.h"

namespace golias {

    namespace {

        struct InstancingKey {
            const Mesh* MeshPtr        = nullptr;
            const Shader* ShaderPtr    = nullptr;
            const Texture* MainTexture = nullptr;
            const Texture* NormalMap   = nullptr;
            uint32_t RenderStateBits   = 0;

            bool operator<(const InstancingKey& other) const {
                if (MeshPtr != other.MeshPtr) {
                    return std::less<const Mesh*>{}(MeshPtr, other.MeshPtr);
                }

                if (ShaderPtr != other.ShaderPtr) {
                    return std::less<const Shader*>{}(ShaderPtr, other.ShaderPtr);
                }

                if (MainTexture != other.MainTexture) {
                    return std::less<const Texture*>{}(MainTexture, other.MainTexture);
                }

                if (NormalMap != other.NormalMap) {
                    return std::less<const Texture*>{}(NormalMap, other.NormalMap);
                }

                return RenderStateBits < other.RenderStateBits;
            }
        };

        uint32_t encoder_renderstate_bits(const RenderState& state) {
            return (static_cast<uint32_t>(state.Blend) << 0) | (static_cast<uint32_t>(state.Cull) << 4) | (state.DepthTest ? (1u << 8) : 0u)
                 | (state.DepthWrite ? (1u << 9) : 0u);
        }

        InstancingKey make_instancing_key(const RenderCommand& command) {
            InstancingKey key;
            key.MeshPtr         = command.Mesh;
            key.ShaderPtr       = nullptr;
            key.MainTexture     = nullptr;
            key.NormalMap       = nullptr;
            key.RenderStateBits = 0;

            if (command.Material) {
                key.ShaderPtr       = command.Material->GetShader().get();
                key.MainTexture     = command.Material->GetTextureParameter("_MainTexture").get();
                key.NormalMap       = command.Material->GetTextureParameter("_NormalMap").get();
                key.RenderStateBits = encoder_renderstate_bits(command.Material->GetRenderState());
            }

            return key;
        }

    } // namespace

    bool OpaqueGeometryPass::Setup() {
        mDefaultShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/default.gshader");
        if (!mDefaultShader) {
            GOLIAS_LOG_ERROR("Failed to create default 3D shader program");
            return false;
        }
        mDefaultShader->SetUniformBlockBinding(GpuLayout::LightingBlock, GpuLayout::LightingBinding);
        mDefaultShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        mDefaultShader->SetUniformBlockBinding(GpuLayout::JointsBlock, GpuLayout::JointsBinding);
        mDefaultShader->SetUniformBlockBinding(GpuLayout::ObjectBlock, GpuLayout::ObjectBinding);
        mDefaultShader->SetUniformBlockBinding(GpuLayout::MaterialBlock, GpuLayout::MaterialBinding);

        // clang-format off
        BufferDesc desc = {
            .Target = BufferTarget::Vertex,
            .Usage = BufferUsage::Dynamic,
            .Size = kMaxInstancesPerBatch * sizeof(InstanceData)
        };
        // clang-format on

        mInstanceBuffer = Engine::GetInstance().GetGraphicsDevice().CreateBuffer(desc);
        return mInstanceBuffer != nullptr;
    }

    void OpaqueGeometryPass::CategorizeRenderCommands(FrameContext& ctx) {
        ctx.Opaque.clear();
        ctx.Transparent.clear();
        ctx.Opaque.reserve(ctx.Commands.size());
        ctx.Transparent.reserve(ctx.Commands.size());

        for (const auto& command : ctx.Commands) {
            if (!command.Mesh) {
                continue;
            }

            if (command.WorldBounds) {

                if (!ctx.Frustum.Intersects(*command.WorldBounds)) {
                    continue;
                }

            } else if (!ctx.Frustum.Intersects(command.Mesh->GetAABB().Transformed(command.Model))) {
                continue;
            }

            const bool isTransparent = command.Material && command.Material->GetRenderState().Blend != BlendMode::None;

            if (isTransparent) {
                ctx.Transparent.push_back(&command);
            } else {
                ctx.Opaque.push_back(&command);
            }
        }
    }

    void OpaqueGeometryPass::DrawInstancedBatches(GraphicsDevice& device, Mesh* mesh, const InstanceData* instanceData, uint32_t instanceCount) {
        // NOTE: The per-instance model matrix comes from the instance buffer itself, so the "model"
        // instancing is active only the instance count / isSkinned flags matter to it.
        uint32_t remaining = instanceCount;
        uint32_t offset    = 0;
        while (remaining > 0) {
            const uint32_t batchSize = std::min<uint32_t>(kMaxInstancesPerBatch, remaining);
            mInstanceBuffer->Update(instanceData + offset, batchSize * sizeof(InstanceData));

            mObjectParams.Update(device, BuildGpuObject(glm::mat4(1.0f), static_cast<int>(batchSize), 0));

            device.BindMesh(mesh);
            mesh->DrawInstanced(mInstanceBuffer, batchSize);
            device.UnbindMesh(mesh);

            offset += batchSize;
            remaining -= batchSize;
        }
    }

    void OpaqueGeometryPass::DrawRenderCommand(
        const RenderCommand& command, FrameContext& ctx, uint32_t instanceCount, const InstanceData* instanceData) {

        GraphicsDevice& device = ctx.Device;

        if (command.Material) {
            device.BindMaterial(command.Material);
            mMaterialParams.Update(device, BuildGpuMaterial(command.Material->GetBaseColor()));

            Shader* shader = command.Material->GetShader().get();

            if (ctx.ShadowTexture) {
                shader->SetTexture(TextureSlots::ShadowMap, ctx.ShadowTexture.get());
            }

            const bool canInstance = instanceCount > 0 && instanceData && shader == mDefaultShader.get();
            if (canInstance) {
                // Instancing is only supported for static (non-skinned) meshes.
                DrawInstancedBatches(device, command.Mesh, instanceData, instanceCount);
                return;
            }

            const bool hasSkin = command.JointMatrices && command.JointCount > 0 && command.JointCount <= kMaxJoints;
            mObjectParams.Update(device, BuildGpuObject(command.Model, 0, hasSkin ? 1 : 0));

            if (hasSkin) {
                mJoints.Update(device, command);
            }
        }

        device.BindMesh(command.Mesh);
        device.DrawMesh(command.Mesh);
        device.UnbindMesh(command.Mesh);
    }

    void OpaqueGeometryPass::RenderInstanced(FrameContext& ctx) {
        if (ctx.Opaque.empty()) {
            return;
        }

        struct InstancingEntry {
            const RenderCommand* Command = nullptr;
            InstancingKey Key            = {};
        };

        // NOTE: Skinned meshes aren't instanced.
        std::vector<InstancingEntry> entries;
        std::vector<const RenderCommand*> skinnedCommands;
        entries.reserve(ctx.Opaque.size());

        for (const RenderCommand* command : ctx.Opaque) {
            if (command->JointMatrices) {
                skinnedCommands.push_back(command);
                continue;
            }

            entries.push_back({command, make_instancing_key(*command)});
        }

        std::sort(entries.begin(), entries.end(), [](const InstancingEntry& a, const InstancingEntry& b) { return a.Key < b.Key; });

        for (size_t i = 0; i < entries.size();) {
            size_t end = i + 1;
            while (end < entries.size() && !(entries[end].Key < entries[i].Key) && !(entries[i].Key < entries[end].Key)) {
                ++end;
            }

            const size_t count         = end - i;
            const RenderCommand* first = entries[i].Command;

            // Instancing is only supported for the default 3D shader.
            const bool canInstance = count >= 2 && first->Material && first->Material->GetShader().get() == mDefaultShader.get();

            if (canInstance) {
                std::vector<InstanceData> instanceData;
                instanceData.reserve(count);
                for (size_t k = i; k < end; ++k) {
                    const RenderCommand* command = entries[k].Command;
                    instanceData.push_back({
                        command->Model,
                        command->Material ? command->Material->GetBaseColor() : glm::vec4(1.0f),
                    });
                }

                DrawRenderCommand(*first, ctx, static_cast<uint32_t>(count), instanceData.data());
            } else {
                for (size_t k = i; k < end; ++k) {
                    DrawRenderCommand(*entries[k].Command, ctx);
                }
            }
            i = end;
        }

        for (const RenderCommand* command : skinnedCommands) {
            DrawRenderCommand(*command, ctx);
        }
    }

    void OpaqueGeometryPass::Execute(FrameContext& ctx) {
        // Cascades (if any) were just built by ShadowPass.
        mFrameParams.Update(ctx.Device, BuildGpuFrame(ctx.Camera, ctx.ShadowMatrices, ctx.ShadowSplits));

        CategorizeRenderCommands(ctx);
        RenderInstanced(ctx);
    }

} // namespace golias
