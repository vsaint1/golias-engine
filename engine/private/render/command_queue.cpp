#include "render/command_queue.h"

#include "core/engine.h"
#include "render/material.h"
#include "render/mesh.h"

namespace golias {

    void CommandQueue::Submit(const RenderCommand& command) {
        mCommands.push_back(command);
    }

    void CommandQueue::Execute() {

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        for (const auto& command : mCommands) {
            if (command.Material) {
                device.BindMaterial(command.Material);
                command.Material->SetParameter("uModel", command.Model);
            }

            if (command.Mesh) {
                command.Mesh->Bind();
                command.Mesh->Draw();
            }
        }

        mCommands.clear();
    }
} // namespace golias
