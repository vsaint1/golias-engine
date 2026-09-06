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
                {0, 2, GL_FLOAT, 0                },
                {1, 2, GL_FLOAT, 2 * sizeof(float)},
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
        mDefault3DShader->SetUniformBlockBinding("Lighting", 0);
        mDefault3DShader->SetUniformBlockBinding("JointMatrices", 2);

        {
            // clang-format off
            BufferDesc desc = {
                .Target = BufferTarget::Vertex,
                .Usage = BufferUsage::Dynamic,
                .Size = kMaxInstancesPerBatch * sizeof(glm::mat4)
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

        mShadowShader->SetUniformBlockBinding("JointMatrices", 2);

        mDefaultUIShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/default_ui.gshader");
        if (!mDefaultUIShader) {
            GOLIAS_LOG_ERROR("Failed to create default UI shader program");
            return false;
        }

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
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (!mLightingBuffer) {

            // clang-format off
            BufferDesc desc = {
                .Target = BufferTarget::Uniform,
                .Usage = BufferUsage::Dynamic,
                .Size = sizeof(GpuLighting)
            };
            // clang-format on

            mLightingBuffer = device.CreateBuffer(desc);
        }

        GpuLighting lighting = {
            .Count = static_cast<int>(std::min(mLightCommands.size(), kMaxLights)),
        };

        for (size_t i = 0; i < static_cast<size_t>(lighting.Count); ++i) {
            const LightCommand& source = mLightCommands[i];
            GpuLight& destination      = lighting.Lights[i];

            destination.Position       = glm::vec4(source.Position, 1.0f);
            destination.Direction      = glm::vec4(source.Direction, 0.0f);
            destination.ColorIntensity = glm::vec4(source.Color, source.Intensity);
            destination.Range          = source.Range;
            destination.SpotAngle      = source.SpotAngle;
            destination.Type           = source.Type;
            destination.IsShadowCaster = source.IsShadowCaster ? 1 : 0;
        }

        mLightingBuffer->Update(&lighting, sizeof(lighting));
        mLightingBuffer->Bind(0);
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
                                         const glm::mat4* instanceMatrices) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (command.Material) {
            command.Material->SetParameterValue("_ModelMatrix", command.Model);
            command.Material->SetParameterValue("_ViewMatrix", cameraCommand.View);
            command.Material->SetParameterValue("_ProjectionMatrix", cameraCommand.Projection);
            command.Material->SetParameterValue("_CameraPos", cameraCommand.CameraPosition);
            device.BindMaterial(command.Material);

            Shader* shader = command.Material->GetShader().get();

            if (mShadowTexture) {
                for (uint32_t cascade = 0; cascade < CascadedShadowMapDesc::kMaxCascades; ++cascade) {
                    shader->SetUniform("_ShadowMatrices[" + std::to_string(cascade) + "]", mShadowCsm.GetCascade(cascade).ViewProjection);
                    shader->SetUniform("_ShadowSplits[" + std::to_string(cascade) + "]", mShadowCsm.GetSplit(cascade));
                }

                shader->SetTexture(TextureSlots::ShadowMap, mShadowTexture.get());
            }

            if (shader == mDefault3DShader.get()) {
                if (command.JointMatrices && command.JointCount > 0 && command.JointCount <= kMaxJoints) {
                    mJointBuffer->Update(command.JointMatrices, command.JointCount * sizeof(glm::mat4));
                    mJointBuffer->Bind(2);
                    shader->SetUniform("_IsSkinned", 1);
                } else {
                    shader->SetUniform("_IsSkinned", 0);
                }
            }

            if (instanceCount > 0 && instanceMatrices && shader == mDefault3DShader.get()) {
                uint32_t remaining = instanceCount;
                uint32_t offset    = 0;
                while (remaining > 0) {
                    const uint32_t batchSize = std::min<uint32_t>(kMaxInstancesPerBatch, remaining);
                    mInstanceBuffer->Update(instanceMatrices + offset, batchSize * sizeof(glm::mat4));
                    
                    shader->SetUniform("_InstanceCount", static_cast<int>(batchSize));

                    device.BindMesh(command.Mesh);
                    command.Mesh->DrawInstanced(*mInstanceBuffer, batchSize);
                    device.UnbindMesh(command.Mesh);

                    offset += batchSize;
                    remaining -= batchSize;
                }

                shader->SetUniform("_InstanceCount", 0);
                return;
            }
        }

        if (command.Material && command.Material->GetShader().get() == mDefault3DShader.get()) {
            command.Material->GetShader()->SetUniform("_InstanceCount", 0);
        }

        device.BindMesh(command.Mesh);
        device.DrawMesh(command.Mesh);
        device.UnbindMesh(command.Mesh);
    }

    void CommandQueue::RenderInstanced(const CameraCommand& cameraCommand, const std::vector<const RenderCommand*>& opaque) {
        if (opaque.empty()) {
            return;
        }

        // Group identical (mesh, material pairs) for instancing. 
        // NOTE: Skinned meshes aren't instanced
        std::vector<const RenderCommand*> staticCommands;
        std::vector<const RenderCommand*> skinnedCommands;

        for (const RenderCommand* command : opaque) {
            
            if (command->JointMatrices) {
                skinnedCommands.push_back(command);
                continue;
            }

            staticCommands.push_back(command);
        }

        std::sort(staticCommands.begin(), staticCommands.end(), [](const RenderCommand* a, const RenderCommand* b) {
            if (a->Mesh != b->Mesh) {
                return std::less<const Mesh*>{}(a->Mesh, b->Mesh);
            }

            return std::less<const Material*>{}(a->Material, b->Material);
        });

        for (size_t i = 0; i < staticCommands.size();) {
            size_t end = i + 1;
            while (end < staticCommands.size() && staticCommands[end]->Mesh == staticCommands[i]->Mesh
                   && staticCommands[end]->Material == staticCommands[i]->Material) {
                ++end;
            }

            const size_t count         = end - i;
            const RenderCommand* first = staticCommands[i];

            // Instancing is only supported for the default 3D shader.
            const bool canInstance = count >= 2 && first->Material && first->Material->GetShader().get() == mDefault3DShader.get();

            if (canInstance) {
                std::vector<glm::mat4> matrices;
                matrices.reserve(count);
                for (size_t k = i; k < end; ++k) {
                    matrices.push_back(staticCommands[k]->Model);
                }

                DrawRenderCommand(*first, cameraCommand, static_cast<uint32_t>(count), matrices.data());
            } else {
                for (size_t k = i; k < end; ++k) {
                    DrawRenderCommand(*staticCommands[k], cameraCommand);
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
            shader->SetUniform("_ModelMatrix", command.Model);
            shader->SetUniform("_ViewMatrix", glm::mat4(1.0f));
            shader->SetUniform("_ProjectionMatrix", cameraCommand.Ortho);
            shader->SetUniform("_Pivot", command.Pivot);
            shader->SetUniform("_Size", command.Size);
            shader->SetUniform("_LowerLeftUV", command.LowerLeftUV);
            shader->SetUniform("_UpperRightUV", command.UpperRightUV);
            shader->SetUniform("_BaseColor", command.Color);
            shader->SetTexture(TextureSlots::MainTexture, command.Texture);
            mQuadMesh->Draw();
        }

        mQuadMesh->Unbind();
    }

    void CommandQueue::RenderCanvas(const CameraCommand& cameraCommand) {
        for (const auto& command : mCanvasCommands) {
            if (!command.Mesh || command.Batches.empty()) {
                continue;
            }

            command.Mesh->Bind();
            mDefaultUIMaterial->Bind();
            Shader* shader = mDefaultUIMaterial->GetShader().get();
            shader->SetUniform("_ProjectionMatrix", cameraCommand.Ortho);

            uint32_t indexOffset = 0;
            for (const CanvasBatch& batch : command.Batches) {

                if (batch.IndexCount > 0) {
                    FrameStats::RecordCanvasBatch(1);

                    Ref<Texture2D> whiteTex = Engine::GetInstance().GetAssetManager().AcquireWhiteTexture();
                    shader->SetTexture(TextureSlots::MainTexture, batch.Texture ? batch.Texture : whiteTex.get());
                    command.Mesh->DrawIndexed(indexOffset, batch.IndexCount);
                }

                indexOffset += batch.IndexCount;
            }

            command.Mesh->Unbind();
        }
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

        mHdrViewport = viewport;
        return true;
    }


    void CommandQueue::RenderPostProcess(const CameraCommand& cameraCommand) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (!mPostProcessShader || !mFullscreenQuad || !mHdrColorTexture) {
            return;
        }


        device.SetDepthTestEnabled(false);
        device.SetBlendMode(BlendMode::None);

        // Tonemap
        device.SetViewport(cameraCommand.Viewport);

        mFxaaShader->Bind();
        mFxaaShader->SetTexture(TextureSlots::MainTexture, mHdrColorTexture.get());
        mFxaaShader->SetUniform("_TexelSizeX", 1.0f / static_cast<float>(mHdrColorTexture->GetDesc().Width));
        mFxaaShader->SetUniform("_TexelSizeY", 1.0f / static_cast<float>(mHdrColorTexture->GetDesc().Height));
        mFxaaShader->SetUniform("_SubpixelQuality", 0.75f);
        mFxaaShader->SetUniform("_EdgeThreshold", 0.25f);
        mFxaaShader->SetUniform("_EdgeThresholdMin", 0.0625f);

        mFullscreenQuad->Bind();
        mFullscreenQuad->Draw();

        // FXAA
        device.SetViewport(cameraCommand.Viewport);

        mPostProcessShader->Bind();
        mPostProcessShader->SetTexture(TextureSlots::MainTexture, mHdrColorTexture.get());
        mPostProcessShader->SetUniform("_Exposure", 1.0f);
        mPostProcessShader->SetUniform("_Tonemap", static_cast<int>(mTonemap));

        mFullscreenQuad->Draw();
        mFullscreenQuad->Unbind();
    }

    void CommandQueue::DrawShadowOpaque(const CameraCommand& cameraCommand, const glm::mat4& cascadeViewProjection) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        struct ShadowGeometry {
            Mesh* Mesh = nullptr;
            std::vector<const RenderCommand*> Commands;
        };

        std::vector<ShadowGeometry> geometries;
        std::vector<const RenderCommand*> skinned;
        std::unordered_map<const Mesh*, size_t> geometryLookup;

        for (const RenderCommand& command : mCommands) {
            if (!command.Mesh) {
                continue;
            }

            // Skip transparent geometry
            if (command.Material && command.Material->GetRenderState().Blend != BlendMode::None) {
                continue;
            }

            if (command.JointMatrices) {
                skinned.push_back(&command);
                continue;
            }

            size_t geometryIndex = geometries.size();

            if (const auto it = geometryLookup.find(command.Mesh); it != geometryLookup.end()) {
                geometryIndex = it->second;
            } else {
                geometryLookup.emplace(command.Mesh, geometryIndex);
                ShadowGeometry geometry;
                geometry.Mesh = command.Mesh;
                geometries.push_back(std::move(geometry));
            }
            geometries[geometryIndex].Commands.push_back(&command);
        }

        for (const ShadowGeometry& geometry : geometries) {
            if (geometry.Commands.empty()) {
                continue;
            }

            if (geometry.Commands.size() < 2) {
                const RenderCommand& command = *geometry.Commands[0];
                mShadowShader->SetUniform("_ModelMatrix", command.Model);
                mShadowShader->SetUniform("_InstanceCount", 0);
                mShadowShader->SetUniform("_IsSkinned", 0);

                device.BindMesh(command.Mesh);
                device.DrawMesh(command.Mesh);
                device.UnbindMesh(command.Mesh);
                continue;
            }

            std::vector<glm::mat4> matrices;
            matrices.reserve(geometry.Commands.size());
            for (const RenderCommand* command : geometry.Commands) {
                matrices.push_back(command->Model);
            }

            uint32_t remaining = static_cast<uint32_t>(matrices.size());
            uint32_t offset    = 0;
            while (remaining > 0) {
                const uint32_t batchSize = std::min<uint32_t>(kMaxInstancesPerBatch, remaining);
                mInstanceBuffer->Update(matrices.data() + offset, batchSize * sizeof(glm::mat4));

                mShadowShader->SetUniform("_InstanceCount", static_cast<int>(batchSize));
                mShadowShader->SetUniform("_IsSkinned", 0);
                device.BindMesh(geometry.Mesh);
                geometry.Mesh->DrawInstanced(*mInstanceBuffer, batchSize);
                device.UnbindMesh(geometry.Mesh);
                offset += batchSize;
                remaining -= batchSize;
            }
            mShadowShader->SetUniform("_InstanceCount", 0);
            continue;
        }

        for (const RenderCommand* command : skinned) {
            mShadowShader->SetUniform("_ModelMatrix", command->Model);
            mShadowShader->SetUniform("_InstanceCount", 0);

            if (command->JointMatrices && command->JointCount > 0 && command->JointCount <= kMaxJoints) {
                mJointBuffer->Update(command->JointMatrices, command->JointCount * sizeof(glm::mat4));
                mJointBuffer->Bind(2);
                mShadowShader->SetUniform("_IsSkinned", 1);
            } else {
                mShadowShader->SetUniform("_IsSkinned", 0);
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
            mShadowShader->SetUniform("_ViewMatrix", mShadowCsm.GetCascades()[cascade].ViewProjection);

            DrawShadowOpaque(cameraCommand, mShadowCsm.GetCascades()[cascade].ViewProjection);
        }

        mShadowFramebuffer->Unbind();
        device.SetCullMode(CullMode::None);
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
