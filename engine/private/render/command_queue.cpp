#include "render/command_queue.h"

#include "core/engine.h"
#include "graphics/buffer.h"
#include "graphics/shader.h"
#include "graphics/texture_2d.h"
#include "render/csm.h"
#include "render/material.h"
#include "render/mesh.h"
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

    void CommandQueue::DrawRenderCommand(const RenderCommand& command, const CameraCommand& cameraCommand) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (command.Material) {
            command.Material->SetParameterValue("_ModelMatrix", command.Model);
            command.Material->SetParameterValue("_ViewMatrix", cameraCommand.View);
            command.Material->SetParameterValue("_ProjectionMatrix", cameraCommand.Projection);
            command.Material->SetParameterValue("_CameraPos", cameraCommand.CameraPosition);
            device.BindMaterial(command.Material);

            if (mShadowTexture) {
                Shader* shader = command.Material->GetShader().get();
                for (uint32_t cascade = 0; cascade < CascadedShadowMapDesc::kMaxCascades; ++cascade) {
                    shader->SetUniform("_ShadowMatrices[" + std::to_string(cascade) + "]", mShadowCsm.GetCascade(cascade).ViewProjection);
                    shader->SetUniform("_ShadowSplits[" + std::to_string(cascade) + "]", mShadowCsm.GetSplit(cascade));
                }

                shader->SetTexture(TextureSlots::ShadowMap, mShadowTexture.get());
            }
        }

        device.BindMesh(command.Mesh);
        device.DrawMesh(command.Mesh);
        device.UnbindMesh(command.Mesh);
    }

    void CommandQueue::RenderGeometry(const CameraCommand& cameraCommand, const std::vector<const RenderCommand*>& opaque) {
        // Opaque geometry: front-to-back order
        for (const RenderCommand* command : opaque) {
            DrawRenderCommand(*command, cameraCommand);
        }
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

        device.SetViewport(cameraCommand.Viewport);
        device.SetDepthTestEnabled(false);
        device.SetBlendMode(BlendMode::None);

        mPostProcessShader->Bind();
        mPostProcessShader->SetTexture(TextureSlots::MainTexture, mHdrColorTexture.get());
        mPostProcessShader->SetUniform("_Exposure", 1.0f);
        // mPostProcessShader->SetUniform("_Tonemap", static_cast<int>(mTonemap));

        mFullscreenQuad->Bind();
        mFullscreenQuad->Draw();
        mFullscreenQuad->Unbind();
    }

    void CommandQueue::RenderShadowCascades(const CameraCommand& cameraCommand, const LightCommand& light) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        mShadowCsm.Prepare(mShadowCsmDesc);
        mShadowCsm.Build(cameraCommand.View, cameraCommand.Projection, light.Direction, cameraCommand.NearPlane, cameraCommand.FarPlane);

        TextureDesc desc;
        desc.Width  = mShadowCsmDesc.ShadowMapResolution;
        desc.Height = mShadowCsmDesc.ShadowMapResolution;
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
        for (uint32_t cascade = 0; cascade < mShadowCsmDesc.CascadeCount; ++cascade) {
            mShadowFramebuffer->SetDepthAttachment(mShadowTexture, cascade);
            mShadowFramebuffer->Bind();
            device.ClearBuffers(ClearFlag::Depth);
            mShadowShader->SetUniform("_ViewMatrix", mShadowCsm.GetCascades()[cascade].ViewProjection);

            for (const RenderCommand& command : mCommands) {
                if (!command.Mesh) {
                    continue;
                }

                // Skip transparent geometry
                if (command.Material && command.Material->GetRenderState().Blend != BlendMode::None) {
                    continue;
                }

                mShadowShader->SetUniform("_ModelMatrix", command.Model);

                device.BindMesh(command.Mesh);
                device.DrawMesh(command.Mesh);
                device.UnbindMesh(command.Mesh);
            }
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
