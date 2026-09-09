#include "render/command_queue.h"

#include "core/engine.h"
#include "graphics/buffer.h"
#include "graphics/query.h"
#include "graphics/shader.h"
#include "graphics/texture_2d.h"
#include "render/csm.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/render_stats.h"
#include "scene/components/camera_component.h"

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

        void UpdateUniformBuffer(Ref<Buffer>& buffer, size_t size, const void* data, uint32_t binding) {
            if (!buffer) {
                BufferDesc desc = {.Target = BufferTarget::Uniform, .Usage = BufferUsage::Dynamic, .Size = size};
                buffer          = Engine::GetInstance().GetGraphicsDevice().CreateBuffer(desc);
            }

            buffer->Update(data, static_cast<uint32_t>(size));
            buffer->Bind(binding);
        }
    } // namespace

    CommandQueue::CommandQueue() {
    }

    CommandQueue::~CommandQueue() {
    }

    bool CommandQueue::Initialize() {

        mQuadMesh = Mesh::CreateQuad();

        mPostProcessShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/postprocess.gshader");
        if (!mPostProcessShader) {
            GOLIAS_LOG_ERROR("Failed to create post-process shader program");
            return false;
        }

        // Full screen quad in normalized device coordinates (NDC)
        // TODO: We can move this to shader
        {

            // clang-format off
            const std::vector<float> vertices = {
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,
            };
            // clang-format on

            const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

            VertexLayout layout;
            layout.Elements = {
                {0, VertexFormat::Float2, 0                },
                {1, VertexFormat::Float2, 2 * sizeof(float)},
            };

            layout.Stride   = 4 * sizeof(float);
            mFullscreenQuad = std::make_shared<Mesh>(layout, vertices, indices);
        }

        mShadowShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/csm.gshader");

        if (!mShadowShader) {
            GOLIAS_LOG_ERROR("Failed to create shadow shader program");
            return false;
        }

        mDefault2DShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/default_2d.gshader");
        if (!mDefault2DShader) {
            GOLIAS_LOG_ERROR("Failed to create default 2D shader program");
            return false;
        }

        mDefault3DShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/default.gshader");
        if (!mDefault3DShader) {
            GOLIAS_LOG_ERROR("Failed to create default 3D shader program");
            return false;
        }
        mDefault3DShader->SetUniformBlockBinding(GpuLayout::LightingBlock, GpuLayout::LightingBinding);
        mDefault3DShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        mDefault3DShader->SetUniformBlockBinding(GpuLayout::JointsBlock, GpuLayout::JointsBinding);
        mDefault3DShader->SetUniformBlockBinding(GpuLayout::ObjectBlock, GpuLayout::ObjectBinding);
        mDefault3DShader->SetUniformBlockBinding(GpuLayout::MaterialBlock, GpuLayout::MaterialBinding);

        {
            // clang-format off
            BufferDesc desc = {
                .Target = BufferTarget::Vertex,
                .Usage = BufferUsage::Dynamic,
                .Size = kMaxInstancesPerBatch * sizeof(InstanceData)
            };
            // clang-format on

            mInstanceBuffer = Engine::GetInstance().GetGraphicsDevice().CreateBuffer(desc);
        }

        {
            // clang-format off
            BufferDesc desc = {
                .Target = BufferTarget::Uniform,
                .Usage = BufferUsage::Dynamic,
                .Size = kMaxJoints * sizeof(glm::mat4)
            };
            // clang-format on

            mJointBuffer = Engine::GetInstance().GetGraphicsDevice().CreateBuffer(desc);
        }

        mShadowShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        mShadowShader->SetUniformBlockBinding(GpuLayout::JointsBlock, GpuLayout::JointsBinding);
        mShadowShader->SetUniformBlockBinding(GpuLayout::ObjectBlock, GpuLayout::ObjectBinding);

        mDefaultUIShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/default_ui.gshader");
        if (!mDefaultUIShader) {
            GOLIAS_LOG_ERROR("Failed to create default UI shader program");
            return false;
        }
        mDefault2DShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        mDefault2DShader->SetUniformBlockBinding(GpuLayout::ObjectBlock, GpuLayout::ObjectBinding);
        mDefault2DShader->SetUniformBlockBinding(GpuLayout::MaterialBlock, GpuLayout::MaterialBinding);

        mDefaultUIShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);

        mDefault2DMaterial = std::make_shared<Material>();
        mDefault2DMaterial->SetShader(mDefault2DShader);
        mDefault2DMaterial->SetDepthTestEnabled(false);
        mDefault2DMaterial->SetBlendMode(BlendMode::Alpha);

        mDefaultUIMaterial = std::make_shared<Material>();
        mDefaultUIMaterial->SetShader(mDefaultUIShader);
        mDefaultUIMaterial->SetDepthTestEnabled(false);
        mDefaultUIMaterial->SetBlendMode(BlendMode::Alpha);

        mFxaaShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/fxaa.gshader");
        if (!mFxaaShader) {
            GOLIAS_LOG_ERROR("Failed to create FXAA shader program");
            return false;
        }
        mPostProcessShader->SetUniformBlockBinding(GpuLayout::PostProcessBlock, GpuLayout::PostProcessBinding);
        mFxaaShader->SetUniformBlockBinding(GpuLayout::PostProcessBlock, GpuLayout::PostProcessBinding);

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();
        if (device.IsQuerySupported()) {
            mActiveGpuQuery  = device.CreateQuery(QueryType::TimeElapsed);
            mStandbyGpuQuery = device.CreateQuery(QueryType::TimeElapsed);
        }

        return true;
    }

    void CommandQueue::Submit(const RenderCommand2D& command) {
        mCommands2D.push_back(command);
    }

    void CommandQueue::Submit(const RenderCommand& command) {
        mCommands.push_back(command);
    }

    void CommandQueue::Submit(const CameraCommand& command) {
        mCameraCommands.push_back(command);
    }

    void CommandQueue::Submit(const RenderCanvasCommand& command) {
        mCanvasCommands.push_back(command);
    }

    void CommandQueue::Submit(const LightCommand& command) {
        if (mLightCommands.size() >= kMaxLights) {
            return;
        }

        mLightCommands.push_back(command);
    }

    void CommandQueue::BeginFrame() {
        FrameStats::BeginFrame();
    }

    void CommandQueue::UpdateLightingBuffer() {
        GpuLighting lighting = {
            .Count = static_cast<int>(std::min(mLightCommands.size(), kMaxLights)),
        };

        for (size_t i = 0; i < static_cast<size_t>(lighting.Count); ++i) {
            const LightCommand& source = mLightCommands[i];
            GpuLight& destination      = lighting.Lights[i];

            destination.Position = glm::vec4(source.Position, 1.0f);
            const glm::vec3 direction =
                glm::length2(source.Direction) > 1e-8f ? glm::normalize(source.Direction) : glm::vec3(0.0f, -1.0f, 0.0f);
            destination.DirectionInvRange = glm::vec4(direction, 1.0f / std::max(source.Range, 1e-6f));
            destination.ColorIntensity    = glm::vec4(source.Color, source.Intensity);
            destination.SpotCutoff        = std::cos(glm::radians(source.SpotAngle));
            destination.Type              = source.Type;
            destination.IsShadowCaster    = source.IsShadowCaster ? 1 : 0;
        }

        UpdateUniformBuffer(mLightingBuffer, sizeof(lighting), &lighting, GpuLayout::LightingBinding);
    }

    void CommandQueue::UpdateFrameBuffer(const CameraCommand& cameraCommand) {
        GpuFrame frame       = {};
        frame.View           = cameraCommand.View;
        frame.Projection     = cameraCommand.Projection;
        frame.Ortho          = cameraCommand.Ortho;
        frame.CameraPosition = glm::vec4(cameraCommand.CameraPosition, 1.0f);
        for (uint32_t cascade = 0; cascade < kMaxShadowCascades; ++cascade) {
            frame.ShadowMatrices[cascade] = mShadowCsm.GetCascade(cascade).ViewProjection;
            frame.ShadowSplits[cascade]   = mShadowCsm.GetSplit(cascade);
        }

        UpdateUniformBuffer(mFrameBuffer, sizeof(frame), &frame, GpuLayout::FrameBinding);
    }

    void CommandQueue::UpdateObjectBuffer(
        const glm::mat4& model, int instanceCount, int isSkinned, const glm::vec4& spritePivotSize, const glm::vec4& spriteUvBounds) {
        GpuObject object       = {};
        object.Model           = model;
        object.Flags           = glm::ivec4(instanceCount, isSkinned, 0, 0);
        object.SpritePivotSize = spritePivotSize;
        object.SpriteUvBounds  = spriteUvBounds;
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        object.NormalMatrix[0] = glm::vec4(normalMatrix[0], 0.0f);
        object.NormalMatrix[1] = glm::vec4(normalMatrix[1], 0.0f);
        object.NormalMatrix[2] = glm::vec4(normalMatrix[2], 0.0f);

        UpdateUniformBuffer(mObjectBuffer, sizeof(object), &object, GpuLayout::ObjectBinding);
    }

    void CommandQueue::UpdateShadowViewProjection(const glm::mat4& viewProjection) {
        mFrameBuffer->Update(&viewProjection, sizeof(viewProjection), static_cast<uint32_t>(offsetof(GpuFrame, ShadowViewProjection)));
        mFrameBuffer->Bind(GpuLayout::FrameBinding);
    }

    void CommandQueue::UpdateMaterialBuffer(const glm::vec4& baseColor) {
        GpuMaterial material = {.BaseColor = baseColor};
        UpdateUniformBuffer(mMaterialBuffer, sizeof(material), &material, GpuLayout::MaterialBinding);
    }

    void CommandQueue::UpdatePostProcessBuffer(const GpuPostProcess& postProcess) {
        UpdateUniformBuffer(mPostProcessBuffer, sizeof(postProcess), &postProcess, GpuLayout::PostProcessBinding);
    }

    void CommandQueue::CategorizeRenderCommands(const Frustum& frustum,
                                                std::vector<const RenderCommand*>& outOpaque,
                                                std::vector<const RenderCommand*>& outTransparent) const {
        outOpaque.clear();
        outTransparent.clear();
        outOpaque.reserve(mCommands.size());
        outTransparent.reserve(mCommands.size());

        for (const auto& command : mCommands) {
            if (!command.Mesh || !frustum.Intersects(command.Mesh->GetAABB().Transformed(command.Model))) {
                continue;
            }

            const bool isTransparent = command.Material && command.Material->GetRenderState().Blend != BlendMode::None;

            if (isTransparent) {
                outTransparent.push_back(&command);
                continue;
            } else {
                outOpaque.push_back(&command);
                continue;
            }
        }
    }

    void CommandQueue::DrawRenderCommand(const RenderCommand& command,
                                         const CameraCommand& cameraCommand,
                                         uint32_t instanceCount,
                                         const InstanceData* instanceData) {

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (command.Material) {
            device.BindMaterial(command.Material);
            const bool hasSkin = command.JointMatrices && command.JointCount > 0 && command.JointCount <= kMaxJoints;
            UpdateObjectBuffer(command.Model, 0, hasSkin ? 1 : 0);
            UpdateMaterialBuffer(command.Material->GetBaseColor());

            Shader* shader = command.Material->GetShader().get();

            if (mShadowTexture) {
                for (uint32_t cascade = 0; cascade < CascadedShadowMapDesc::kMaxCascades; ++cascade) {
                }

                shader->SetTexture(TextureSlots::ShadowMap, mShadowTexture.get());
            }

            if (hasSkin) {
                mJointBuffer->Update(command.JointMatrices, command.JointCount * sizeof(glm::mat4));
                mJointBuffer->Bind(GpuLayout::JointsBinding);
            }

            if (instanceCount > 0 && instanceData && shader == mDefault3DShader.get()) {
                uint32_t remaining = instanceCount;
                uint32_t offset    = 0;
                while (remaining > 0) {
                    const uint32_t batchSize = std::min<uint32_t>(kMaxInstancesPerBatch, remaining);
                    mInstanceBuffer->Update(instanceData + offset, batchSize * sizeof(InstanceData));

                    UpdateObjectBuffer(command.Model, static_cast<int>(batchSize), 0);

                    device.BindMesh(command.Mesh);
                    command.Mesh->DrawInstanced(mInstanceBuffer, batchSize);
                    device.UnbindMesh(command.Mesh);

                    offset += batchSize;
                    remaining -= batchSize;
                }

                return;
            }
        }

        device.BindMesh(command.Mesh);
        device.DrawMesh(command.Mesh);
        device.UnbindMesh(command.Mesh);
    }

    void CommandQueue::RenderInstanced(const CameraCommand& cameraCommand, const std::vector<const RenderCommand*>& opaque) {
        if (opaque.empty()) {
            return;
        }

        struct InstancingEntry {
            const RenderCommand* Command = nullptr;
            InstancingKey Key            = {};
        };

        // NOTE: Skinned meshes aren't instanced.
        std::vector<InstancingEntry> entries;
        std::vector<const RenderCommand*> skinnedCommands;
        entries.reserve(opaque.size());

        for (const RenderCommand* command : opaque) {
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
            const bool canInstance = count >= 2 && first->Material && first->Material->GetShader().get() == mDefault3DShader.get();

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

                DrawRenderCommand(*first, cameraCommand, static_cast<uint32_t>(count), instanceData.data());
            } else {
                for (size_t k = i; k < end; ++k) {
                    DrawRenderCommand(*entries[k].Command, cameraCommand);
                }
            }
            i = end;
        }

        for (const RenderCommand* command : skinnedCommands) {
            DrawRenderCommand(*command, cameraCommand);
        }
    }

    void CommandQueue::RenderGeometry(const CameraCommand& cameraCommand, const std::vector<const RenderCommand*>& opaque) {
        RenderInstanced(cameraCommand, opaque);
    }

    void CommandQueue::RenderTransparent(const CameraCommand& cameraCommand, std::vector<const RenderCommand*>& transparent) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        // Sort back-to-front (furthest first).
        const glm::vec3 cameraPosition = cameraCommand.CameraPosition;
        std::sort(transparent.begin(), transparent.end(), [&](const RenderCommand* a, const RenderCommand* b) {
            const glm::vec3 centerA = glm::vec3(a->Model * glm::vec4(a->Mesh->GetAABB().GetCenter(), 1.0f));
            const glm::vec3 centerB = glm::vec3(b->Model * glm::vec4(b->Mesh->GetAABB().GetCenter(), 1.0f));
            return glm::distance(cameraPosition, centerA) > glm::distance(cameraPosition, centerB);
        });

        device.SetDepthWriteEnabled(false);

        for (const RenderCommand* command : transparent) {
            DrawRenderCommand(*command, cameraCommand);
        }

        device.SetDepthWriteEnabled(true);
    }

    void CommandQueue::RenderSprites(const CameraCommand& cameraCommand) {
        if (mCommands2D.empty()) {
            return;
        }

        mQuadMesh->Bind();

        for (const auto& command : mCommands2D) {
            mDefault2DMaterial->Bind();
            Shader* shader = mDefault2DMaterial->GetShader().get();
            UpdateObjectBuffer(
                command.Model, 0, 0, glm::vec4(command.Pivot, command.Size), glm::vec4(command.LowerLeftUV, command.UpperRightUV));
            UpdateMaterialBuffer(command.Color);
            shader->SetTexture(TextureSlots::MainTexture, command.Texture);
            mQuadMesh->Draw();
        }

        mQuadMesh->Unbind();
    }

    void CommandQueue::RenderCanvas(const CameraCommand& cameraCommand) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mCanvasCommands.empty()) {
            return;
        }

        for (const auto& command : mCanvasCommands) {
            if (!command.Mesh || command.Batches.empty()) {
                continue;
            }

            command.Mesh->Bind();
            mDefaultUIMaterial->Bind();
            Shader* shader       = mDefaultUIMaterial->GetShader().get();
            uint32_t indexOffset = 0;
            for (const CanvasBatch& batch : command.Batches) {

                if (batch.IndexCount > 0) {
                    FrameStats::RecordCanvasBatch(1);

                    if (batch.HasClip) {

                        ScissorRect scissor = batch.ClipRect.ToScissorRect(command.Viewport);

                        device.SetScissorTestEnabled(true);
                        device.SetScissorRect(scissor);

                    } else {
                        device.SetScissorTestEnabled(false);
                    }

                    Ref<Texture2D> whiteTex = Engine::GetInstance().GetAssetManager().AcquireWhiteTexture();
                    shader->SetTexture(TextureSlots::MainTexture, batch.Texture ? batch.Texture : whiteTex.get());
                    command.Mesh->DrawIndexed(indexOffset, batch.IndexCount);
                }

                indexOffset += batch.IndexCount;
            }

            command.Mesh->Unbind();
        }

        glDisable(GL_SCISSOR_TEST);
    }

    void CommandQueue::Execute() {

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mStandbyGpuQuery) {
            uint64_t nanoseconds = 0;
            if (mStandbyGpuQuery->GetResult(&nanoseconds) == QueryResult::Available) {
                FrameStats::RecordGpuTime(nanoseconds * 1e-6f);
            }
        }

        if (mActiveGpuQuery) {
            mActiveGpuQuery->Begin();
        }

        UpdateLightingBuffer();

        for (const auto& cameraCommand : mCameraCommands) {

            if (!EnsureHdrTargets(cameraCommand.Viewport)) {
                continue;
            }

            const Frustum frustum = Frustum::FromMatrix(cameraCommand.Projection * cameraCommand.View);

            mHdrFramebuffer->Bind();

            device.SetViewport(cameraCommand.Viewport);
            device.SetClearColor();
            device.ClearBuffers(ClearFlag::Color | ClearFlag::Depth);

            for (const LightCommand& light : mLightCommands) {
                if (light.Type == 0 && light.IsShadowCaster && cameraCommand.Shadows.Enabled) {
                    RenderShadowCascades(cameraCommand, light);
                    break;
                }
            }

            // NOTE: The shadow pass unbinds its own framebuffer, so re-bind the HDR target.
            mHdrFramebuffer->Bind();
            UpdateFrameBuffer(cameraCommand);

            // Categorize draw calls frustum-culled into opaque and transparent sets.
            std::vector<const RenderCommand*> opaque;
            std::vector<const RenderCommand*> transparent;
            CategorizeRenderCommands(frustum, opaque, transparent);

            RenderGeometry(cameraCommand, opaque);
            RenderTransparent(cameraCommand, transparent);

            mHdrFramebuffer->Unbind();

            RenderPostProcess(cameraCommand);

            device.SetDepthTestEnabled(false);
            device.SetBlendMode(BlendMode::Alpha);

            RenderSprites(cameraCommand);
            RenderCanvas(cameraCommand);

            device.SetBlendMode(BlendMode::None);
            device.SetDepthTestEnabled(true);
        }

        if (mActiveGpuQuery) {
            mActiveGpuQuery->End();
            std::swap(mActiveGpuQuery, mStandbyGpuQuery);
        }
    }

    bool CommandQueue::EnsureHdrTargets(const Viewport& viewport) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mHdrFramebuffer && viewport.Width == mHdrViewport.Width && viewport.Height == mHdrViewport.Height) {
            return true;
        }

        if (viewport.Width <= 0 || viewport.Height <= 0) {
            return false;
        }

        TextureDesc colorDesc;
        colorDesc.Width  = static_cast<uint32_t>(viewport.Width);
        colorDesc.Height = static_cast<uint32_t>(viewport.Height);
        colorDesc.Layers = 1;
        colorDesc.Format = TextureFormat::RGBA16F;
        colorDesc.Filter = TextureFilter::Linear;
        colorDesc.Wrap   = TextureWrap::ClampToEdge;

        TextureDesc depthDesc;
        depthDesc.Width  = static_cast<uint32_t>(viewport.Width);
        depthDesc.Height = static_cast<uint32_t>(viewport.Height);
        depthDesc.Layers = 1;
        depthDesc.Format = TextureFormat::Depth24;
        depthDesc.Filter = TextureFilter::Nearest;
        depthDesc.Wrap   = TextureWrap::ClampToEdge;

        mHdrColorTexture = device.CreateTexture2D(colorDesc);
        mHdrDepthTexture = device.CreateTexture2DArray(depthDesc);
        mHdrFramebuffer  = device.CreateFramebuffer(colorDesc);

        mHdrFramebuffer->SetColorAttachment(0, mHdrColorTexture);
        mHdrFramebuffer->SetDepthAttachment(mHdrDepthTexture);

        if (!mHdrFramebuffer->IsComplete()) {
            GOLIAS_LOG_ERROR("HDR framebuffer is incomplete.");
            return false;
        }

        if (!EnsureLdrTargets(viewport)) {
            return false;
        }

        mHdrViewport = viewport;
        return true;
    }

    bool CommandQueue::EnsureLdrTargets(const Viewport& viewport) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mLdrFramebuffer && viewport.Width == mLdrViewport.Width && viewport.Height == mLdrViewport.Height) {
            return true;
        }

        if (viewport.Width <= 0 || viewport.Height <= 0) {
            return false;
        }

        // LDR intermediate: tonemapped result is written here before FXAA reads it.
        TextureDesc ldrDesc;
        ldrDesc.Width  = static_cast<uint32_t>(viewport.Width);
        ldrDesc.Height = static_cast<uint32_t>(viewport.Height);
        ldrDesc.Layers = 1;
        ldrDesc.Format = TextureFormat::RGBA8;
        ldrDesc.Filter = TextureFilter::Linear;
        ldrDesc.Wrap   = TextureWrap::ClampToEdge;

        mLdrColorTexture = device.CreateTexture2D(ldrDesc);
        mLdrFramebuffer  = device.CreateFramebuffer(ldrDesc);
        mLdrFramebuffer->SetColorAttachment(0, mLdrColorTexture);

        if (!mLdrFramebuffer->IsComplete()) {
            GOLIAS_LOG_ERROR("LDR framebuffer is incomplete.");
            return false;
        }

        mLdrViewport = viewport;
        return true;
    }


    void CommandQueue::RenderPostProcess(const CameraCommand& cameraCommand) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (!mPostProcessShader || !mFxaaShader || !mFullscreenQuad || !mHdrColorTexture || !mLdrColorTexture) {
            return;
        }

        device.SetDepthTestEnabled(false);
        device.SetBlendMode(BlendMode::None);

        mFullscreenQuad->Bind();

        mLdrFramebuffer->Bind();
        device.SetViewport(cameraCommand.Viewport);

        mPostProcessShader->Bind();
        mPostProcessShader->SetTexture(TextureSlots::MainTexture, mHdrColorTexture.get());
        UpdatePostProcessBuffer({
            .Exposure = 1.0f,
            .Tonemap  = static_cast<int>(mTonemap),
        });

        mFullscreenQuad->Draw();
        mLdrFramebuffer->Unbind();

        device.SetViewport(cameraCommand.Viewport);

        mFxaaShader->Bind();
        mFxaaShader->SetTexture(TextureSlots::MainTexture, mLdrColorTexture.get());
        UpdatePostProcessBuffer({
            .TexelSizeX       = 1.0f / static_cast<float>(mLdrColorTexture->GetDesc().Width),
            .TexelSizeY       = 1.0f / static_cast<float>(mLdrColorTexture->GetDesc().Height),
            .SubpixelQuality  = 0.75f,
            .EdgeThreshold    = 0.25f,
            .EdgeThresholdMin = 0.0625f,
        });

        mFullscreenQuad->Draw();
        mFullscreenQuad->Unbind();
    }

    void CommandQueue::DrawShadowOpaque(const CameraCommand& cameraCommand, const std::vector<const RenderCommand*>& casters) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

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
                UpdateObjectBuffer(command.Model, 0, 0);

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

            uint32_t remaining = static_cast<uint32_t>(instanceData.size());
            uint32_t offset    = 0;
            while (remaining > 0) {
                const uint32_t batchSize = std::min<uint32_t>(kMaxInstancesPerBatch, remaining);
                mInstanceBuffer->Update(instanceData.data() + offset, batchSize * sizeof(InstanceData));

                UpdateObjectBuffer(glm::mat4(1.0f), static_cast<int>(batchSize), 0);
                device.BindMesh(geometry.Mesh);
                geometry.Mesh->DrawInstanced(mInstanceBuffer, batchSize);
                device.UnbindMesh(geometry.Mesh);
                offset += batchSize;
                remaining -= batchSize;
            }
            continue;
        }

        for (const RenderCommand* command : skinned) {
            const bool hasSkin = command->JointMatrices && command->JointCount > 0 && command->JointCount <= kMaxJoints;
            UpdateObjectBuffer(command->Model, 0, hasSkin ? 1 : 0);

            if (hasSkin) {
                mJointBuffer->Update(command->JointMatrices, command->JointCount * sizeof(glm::mat4));
                mJointBuffer->Bind(GpuLayout::JointsBinding);
            }

            device.BindMesh(command->Mesh);
            device.DrawMesh(command->Mesh);
            device.UnbindMesh(command->Mesh);
        }
    }

    void CommandQueue::RenderShadowCascades(const CameraCommand& cameraCommand, const LightCommand& light) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        mShadowCsm.Prepare();
        mShadowCsm.Build(cameraCommand.View, cameraCommand.Projection, light.Direction, cameraCommand.NearPlane, cameraCommand.FarPlane);
        UpdateFrameBuffer(cameraCommand);

        const CascadedShadowMapDesc shadowCsmDesc = mShadowCsm.GetSettings();

        TextureDesc desc;
        desc.Width  = shadowCsmDesc.ShadowMapResolution;
        desc.Height = shadowCsmDesc.ShadowMapResolution;
        desc.Layers = CascadedShadowMapDesc::kMaxCascades;
        desc.Format = TextureFormat::Depth24;
        desc.Filter = TextureFilter::Linear;
        desc.Wrap   = TextureWrap::ClampToBorder;

        if (!mShadowTexture) {
            mShadowTexture     = device.CreateTexture2DArray(desc);
            mShadowFramebuffer = device.CreateFramebuffer(desc);
        } else if (mShadowTexture->GetDesc().Width != desc.Width || mShadowTexture->GetDesc().Height != desc.Height) {
            mShadowTexture->Recreate(desc);
            mShadowFramebuffer = device.CreateFramebuffer(desc);
        }

        mShadowFramebuffer->SetDepthAttachment(mShadowTexture, 0);
        if (!mShadowFramebuffer->IsComplete()) {
            GOLIAS_LOG_ERROR("CSM framebuffer is incomplete.");
            mShadowFramebuffer->Unbind();
            device.SetViewport(cameraCommand.Viewport);
            return;
        }

        mShadowFramebuffer->Bind();

        device.SetDepthTestEnabled(true);
        device.SetCullMode(CullMode::Front);

        mShadowShader->Bind();
        for (uint32_t cascade = 0; cascade < shadowCsmDesc.CascadeCount; ++cascade) {
            mShadowFramebuffer->SetDepthAttachment(mShadowTexture, cascade);
            mShadowFramebuffer->Bind();
            device.ClearBuffers(ClearFlag::Depth);

            const Cascade& cascadeData = mShadowCsm.GetCascades()[cascade];
            UpdateShadowViewProjection(cascadeData.ViewProjection);

            std::vector<const RenderCommand*> cascadeCasters;
            if (mCommands.size() > 0) {
                cascadeCasters.reserve(mCommands.size());
            }
            for (const RenderCommand& command : mCommands) {
                if (!command.Mesh) {
                    continue;
                }

                if (command.Material && command.Material->GetRenderState().Blend != BlendMode::None) {
                    continue;
                }

                if (command.JointMatrices) {
                    cascadeCasters.push_back(&command);
                    continue;
                }

                if (CascadeContains(command.Mesh->GetAABB(), command.Model, cascadeData.ViewProjection)) {
                    cascadeCasters.push_back(&command);
                }
            }

            DrawShadowOpaque(cameraCommand, cascadeCasters);
        }

        mShadowFramebuffer->Unbind();
        UpdateFrameBuffer(cameraCommand);
        device.SetCullMode(CullMode::Back);
        device.SetViewport(cameraCommand.Viewport);
    }

    void CommandQueue::EndFrame() {
        mCommands.clear();
        mCameraCommands.clear();
        mLightCommands.clear();
        mCanvasCommands.clear();
        mCommands2D.clear();
    }
} // namespace golias
