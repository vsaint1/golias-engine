#include "render/command_queue.h"

#include "core/engine.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/components/camera_component.h"

namespace golias {

    void CommandQueue::Submit(const RenderCommand& command) {
        mCommands.push_back(command);
    }

    void CommandQueue::Submit(const CameraCommand& command) {
        mCameraCommands.push_back(command);
    }

    void CommandQueue::BeginFrame() {
    }

    void CommandQueue::Execute() {


        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();


        for (const auto& cameraCommand : mCameraCommands) {

            device.SetViewport(cameraCommand.Viewport);

            for (const auto& command : mCommands) {
                if (command.Material) {
                    device.BindMaterial(command.Material);
                    command.Material->SetParameter("uModel", command.Model);
                    command.Material->SetParameter("uView", cameraCommand.View);
                    command.Material->SetParameter("uProjection", cameraCommand.Projection);
                }

                if (command.Mesh) {
                    command.Mesh->Bind();
                    command.Mesh->Draw();
                }
            }
        }
    }

    void CommandQueue::EndFrame() {
        mCommands.clear();
        mCameraCommands.clear();
    }
} // namespace golias
