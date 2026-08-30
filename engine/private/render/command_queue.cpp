#include "render/command_queue.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/csm.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/components/camera_component.h"

namespace golias {

    CommandQueue::CommandQueue() {
    }

    CommandQueue::~CommandQueue() {
        if (mLightingBuffer) {
            Engine::GetInstance().GetGraphicsDevice().DestroyBuffer(mLightingBuffer);
        }
    }

    bool CommandQueue::Initialize() {

        mQuadMesh = Mesh::CreateQuad();

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

    void CommandQueue::Execute() {


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
            device.BindUniformBuffer(mLightingBuffer, 0);
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

        device.UpdateBuffer(mLightingBuffer, BufferTarget::Uniform, &lighting, sizeof(lighting));

        for (const auto& cameraCommand : mCameraCommands) {

            device.SetViewport(cameraCommand.Viewport);
            const Frustum frustum = Frustum::FromMatrix(cameraCommand.Projection * cameraCommand.View);
            mHasShadows           = false;

            for (const LightCommand& light : mLightCommands) {
                if (light.Type == 0 && light.IsShadowCaster && cameraCommand.Shadows.Enabled) {
                    RenderShadowCascades(cameraCommand, light);
                    break;
                }
            }

            for (const auto& command : mCommands) {
                if (!command.Mesh || !frustum.Intersects(command.Mesh->GetAABB().Transformed(command.Model))) {
                    continue;
                }
                if (command.Material) {
                    command.Material->SetParameter("_ModelMatrix", command.Model);
                    command.Material->SetParameter("_ViewMatrix", cameraCommand.View);
                    command.Material->SetParameter("_ProjectionMatrix", cameraCommand.Projection);
                    command.Material->SetParameter("_CameraPos", cameraCommand.CameraPosition);
                    device.BindMaterial(command.Material);
                    if (mHasShadows) {
                        Shader* shader = command.Material->GetShader().get();
                        for (uint32_t cascade = 0; cascade < CascadedShadowMapDesc::kMaxCascades; ++cascade) {
                            shader->SetUniform("_ShadowMatrices[" + std::to_string(cascade) + "]",
                                               mShadowCsm.GetCascade(cascade).ViewProjection);
                            shader->SetUniform("_ShadowSplits[" + std::to_string(cascade) + "]", mShadowCsm.GetSplit(cascade));
                        }

                        shader->SetTexture(TextureSlots::ShadowMap, mShadowTexture.get());
                    }
                }

                device.BindMesh(command.Mesh);
                device.DrawMesh(command.Mesh);
                device.UnbindMesh(command.Mesh);
            }


            device.SetDepthTestEnabled(false);
            device.SetBlendMode(BlendMode::Alpha);
            mQuadMesh->Bind();

            for (const auto& command : mCommands2D) {
                mDefault2DShader->Bind();
                mDefault2DShader->SetUniform("_ModelMatrix", command.Model);
                mDefault2DShader->SetUniform("_ViewMatrix", glm::mat4(1.0f));
                mDefault2DShader->SetUniform("_ProjectionMatrix", cameraCommand.Ortho);
                mDefault2DShader->SetUniform("_Pivot", command.Pivot);
                mDefault2DShader->SetUniform("_Size", command.Size);
                mDefault2DShader->SetUniform("_LowerLeftUV", command.LowerLeftUV);
                mDefault2DShader->SetUniform("_UpperRightUV", command.UpperRightUV);
                mDefault2DShader->SetUniform("_BaseColor", command.Color);
                mDefault2DShader->SetTexture(TextureSlots::MainTexture, command.Texture);
                mQuadMesh->Draw();
            }

            mQuadMesh->Unbind();

            for (const auto& command : mCanvasCommands) {
                if (!command.Mesh || command.Batches.empty()) {
                    continue;
                }

                command.Mesh->Bind();
                mDefaultUIShader->Bind();
                mDefaultUIShader->SetUniform("_ProjectionMatrix", cameraCommand.Ortho);

                uint32_t indexOffset = 0;
                for (const CanvasBatch& batch : command.Batches) {

                    if (batch.Texture && batch.IndexCount > 0) {
                        mDefaultUIShader->SetTexture(TextureSlots::MainTexture, batch.Texture);
                        command.Mesh->DrawIndexed(indexOffset, batch.IndexCount);
                    }

                    indexOffset += batch.IndexCount;
                }

                command.Mesh->Unbind();
            }

            device.SetBlendMode(BlendMode::None);
            device.SetDepthTestEnabled(true);
        }
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

                mShadowShader->SetUniform("_ModelMatrix", command.Model);

                device.BindMesh(command.Mesh);
                device.DrawMesh(command.Mesh);
                device.UnbindMesh(command.Mesh);
            }
        }

        mShadowFramebuffer->Unbind();
        device.SetCullMode(CullMode::None);
        device.SetViewport(cameraCommand.Viewport);

        mHasShadows = true;
    }

    void CommandQueue::EndFrame() {
        mCommands.clear();
        mCameraCommands.clear();
        mLightCommands.clear();
        mCanvasCommands.clear();
        mCommands2D.clear();
    }
} // namespace golias
