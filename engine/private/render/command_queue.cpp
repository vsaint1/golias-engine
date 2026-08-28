#include "render/command_queue.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/csm.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/components/camera_component.h"

namespace golias {

    CommandQueue::~CommandQueue() {
        if (mLightingBuffer) {
            Engine::GetInstance().GetGraphicsDevice().DestroyBuffer(mLightingBuffer);
        }
    }

    void CommandQueue::Submit(const RenderCommand& command) {
        mCommands.push_back(command);
    }

    void CommandQueue::Submit(const CameraCommand& command) {
        mCameraCommands.push_back(command);
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
            mLightingBuffer = device.CreateUniformBuffer(sizeof(GpuLighting));
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

        device.UpdateUniformBuffer(mLightingBuffer, &lighting, sizeof(lighting));

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
        }
    }

    void CommandQueue::RenderShadowCascades(const CameraCommand& cameraCommand, const LightCommand& light) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        mShadowCsm.Prepare(mShadowCsmDesc);
        mShadowCsm.Build(cameraCommand.View, cameraCommand.Projection, light.Direction, cameraCommand.NearPlane, cameraCommand.FarPlane);

        if (!mShadowShader) {
            mShadowShader = Engine::GetInstance().GetAssetManager().Load<Shader>("shaders/csm.gshader");
        }

        if (!mShadowShader) {
            return;
        }

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
    }
} // namespace golias
