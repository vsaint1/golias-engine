#include "render/command_queue.h"

#include "core/engine.h"
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

            for (const auto& command : mCommands) {
                if (command.Material) {
                    command.Material->SetParameter("_ModelMatrix", command.Model);
                    command.Material->SetParameter("_ViewMatrix", cameraCommand.View);
                    command.Material->SetParameter("_ProjectionMatrix", cameraCommand.Projection);
                    command.Material->SetParameter("_CameraPos", cameraCommand.CameraPosition);
                    device.BindMaterial(command.Material);
                }

                device.BindMesh(command.Mesh);
                device.DrawMesh(command.Mesh);
                device.UnbindMesh(command.Mesh);
            }
        }
    }

    void CommandQueue::EndFrame() {
        mCommands.clear();
        mCameraCommands.clear();
        mLightCommands.clear();
    }
} // namespace golias
