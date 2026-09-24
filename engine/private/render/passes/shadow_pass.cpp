#include "render/passes/shadow_pass.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/frame_params.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/passes/opaque_geometry_pass.h"
#include "render/render_targets.h"

namespace golias {

    ShadowPass::ShadowPass(RenderTargets& targets, OpaqueGeometryPass& geometryPass) : mTargets(targets), mGeometryPass(geometryPass) {
    }

    bool ShadowPass::Setup() {
        mShadowShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/csm.gshader");
        if (!mShadowShader) {
            GOLIAS_LOG_ERROR("Failed to create shadow shader program");
            return false;
        }

        mShadowShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        mShadowShader->SetUniformBlockBinding(GpuLayout::JointsBlock, GpuLayout::JointsBinding);
        mShadowShader->SetUniformBlockBinding(GpuLayout::ObjectBlock, GpuLayout::ObjectBinding);
        return true;
    }

    void ShadowPass::DrawShadowOpaque(FrameContext& ctx, const std::vector<const RenderCommand*>& casters) {
        GraphicsDevice& device = ctx.Device;

        struct ShadowGeometry {
            Mesh* Mesh = nullptr;
            std::vector<const RenderCommand*> Commands;
        };

        std::vector<ShadowGeometry> geometries;
        std::vector<const RenderCommand*> skinned;
        std::unordered_map<const Mesh*, size_t> geometryLookup;

        for (const RenderCommand* command : casters) {
            if (!command->Mesh) {
                continue;
            }

            // Skip transparent geometry
            if (command->Material && command->Material->GetRenderState().Blend != BlendMode::None) {
                continue;
            }

            if (command->JointMatrices) {
                skinned.push_back(command);
                continue;
            }

            size_t geometryIndex = geometries.size();

            if (const auto it = geometryLookup.find(command->Mesh); it != geometryLookup.end()) {
                geometryIndex = it->second;
            } else {
                geometryLookup.emplace(command->Mesh, geometryIndex);
                ShadowGeometry geometry;
                geometry.Mesh = command->Mesh;
                geometries.push_back(std::move(geometry));
            }
            geometries[geometryIndex].Commands.push_back(command);
        }

        for (const ShadowGeometry& geometry : geometries) {
            if (geometry.Commands.empty()) {
                continue;
            }

            if (geometry.Commands.size() < 2) {
                const RenderCommand& command = *geometry.Commands[0];
                mObjectParams.Update(device, BuildGpuObject(command.Model, 0, 0));

                device.BindMesh(command.Mesh);
                device.DrawMesh(command.Mesh);
                device.UnbindMesh(command.Mesh);
                continue;
            }

            std::vector<InstanceData> instanceData;
            instanceData.reserve(geometry.Commands.size());
            for (const RenderCommand* command : geometry.Commands) {
                instanceData.push_back({command->Model, glm::vec4(1.0f)});
            }

            mGeometryPass.DrawInstancedBatches(device, geometry.Mesh, instanceData.data(), static_cast<uint32_t>(instanceData.size()));
        }

        for (const RenderCommand* command : skinned) {
            const bool hasSkin = command->JointMatrices && command->JointCount > 0 && command->JointCount <= kMaxJoints;
            mObjectParams.Update(device, BuildGpuObject(command->Model, 0, hasSkin ? 1 : 0));

            if (hasSkin) {
                mJoints.Update(device, *command);
            }

            device.BindMesh(command->Mesh);
            device.DrawMesh(command->Mesh);
            device.UnbindMesh(command->Mesh);
        }
    }

    void ShadowPass::RenderShadowCascades(FrameContext& ctx, const LightCommand& light) {
        GraphicsDevice& device      = ctx.Device;
        const CameraCommand& camera = ctx.Camera;

        mShadowCsm.Prepare();
        mShadowCsm.Build(camera.View, camera.Projection, light.Direction, camera.NearPlane, camera.FarPlane);

        std::array<glm::mat4, kMaxShadowCascades> cascadeMatrices = {};
        std::array<float, kMaxShadowCascades> cascadeSplits       = {};
        for (uint32_t cascade = 0; cascade < kMaxShadowCascades; ++cascade) {
            cascadeMatrices[cascade] = mShadowCsm.GetCascade(cascade).ViewProjection;
            cascadeSplits[cascade]   = mShadowCsm.GetSplit(cascade);
        }
        mFrameParams.Update(device, BuildGpuFrame(camera, cascadeMatrices, cascadeSplits));

        const CascadedShadowMapDesc shadowCsmDesc = mShadowCsm.GetSettings();

        if (!mTargets.EnsureShadow(shadowCsmDesc.ShadowMapResolution, device.GetDepthTextureFormat())) {
            return;
        }

        Framebuffer* shadowFramebuffer           = mTargets.GetShadowFramebuffer();
        const Ref<Texture2DArray>& shadowTexture = mTargets.GetShadowTexture();

        shadowFramebuffer->SetDepthAttachment(shadowTexture, 0);
        if (!shadowFramebuffer->IsComplete()) {
            GOLIAS_LOG_ERROR("CSM framebuffer is incomplete.");
            shadowFramebuffer->Unbind();
            device.SetCullMode(CullMode::Back);
            device.SetViewport(camera.Viewport);
            return;
        }

        shadowFramebuffer->Bind();

        device.SetDepthTestEnabled(true);
        device.SetCullMode(CullMode::Front);

        mShadowShader->Bind();
        for (uint32_t cascade = 0; cascade < shadowCsmDesc.CascadeCount; ++cascade) {
            shadowFramebuffer->SetDepthAttachment(shadowTexture, cascade);
            shadowFramebuffer->Bind();
            device.ClearBuffers(ClearFlag::Depth);

            const Cascade& cascadeData = mShadowCsm.GetCascades()[cascade];
            mFrameParams.UpdatePartial(
                device, &cascadeData.ViewProjection, sizeof(cascadeData.ViewProjection), offsetof(GpuFrame, ShadowViewProjection));

            std::vector<const RenderCommand*> cascadeCasters;
            if (ctx.Commands.size() > 0) {
                cascadeCasters.reserve(ctx.Commands.size());
            }

            for (const RenderCommand& command : ctx.Commands) {
                if (!command.Mesh) {
                    continue;
                }

                if (command.Material && command.Material->GetRenderState().Blend != BlendMode::None) {
                    continue;
                }

                // NOTE: Skinned meshes are bounds-tested against their own precomputed world-space
                // bounds (their vertices move with the skeleton, so the mesh's bind-pose AABB combined
                // with the root transform is not a valid bound). Static meshes fall through to the
                // model-space AABB test below. A command must only ever be added to cascadeCasters once
                // - do NOT let a skinned command fall through into the static-mesh test too, or it will
                // be queued (and drawn) twice for this cascade.
                if (command.JointMatrices) {
                    if (command.WorldBounds && CascadeContains(*command.WorldBounds, glm::mat4(1.0f), cascadeData.ViewProjection)) {
                        cascadeCasters.push_back(&command);
                    }

                    continue;
                }

                if (CascadeContains(command.Mesh->GetAABB(), command.Model, cascadeData.ViewProjection)) {
                    cascadeCasters.push_back(&command);
                }
            }

            DrawShadowOpaque(ctx, cascadeCasters);
        }

        shadowFramebuffer->Unbind();
        device.SetCullMode(CullMode::Back);
        device.SetViewport(camera.Viewport);

        ctx.ShadowTexture = shadowTexture;
    }

    void ShadowPass::Execute(FrameContext& ctx) {
        for (const LightCommand& light : ctx.Lights) {
            if (light.Type == 0 && light.IsShadowCaster && ctx.Camera.Shadows.Enabled) {
                RenderShadowCascades(ctx, light);
                break;
            }
        }

        // Publish the cascade (possibly stale, from a previous frame if no light cast shadows this frame) 
        for (uint32_t cascade = 0; cascade < kMaxShadowCascades; ++cascade) {
            ctx.ShadowMatrices[cascade] = mShadowCsm.GetCascade(cascade).ViewProjection;
            ctx.ShadowSplits[cascade]   = mShadowCsm.GetSplit(cascade);
        }

        // NOTE: The shadow pass unbinds its own framebuffer, so re-bind the HDR target for the passes that follow (opaque/transparent/unlit geometry).
        ctx.RenderTargets->GetHdrFramebuffer()->Bind();
    }

} // namespace golias
