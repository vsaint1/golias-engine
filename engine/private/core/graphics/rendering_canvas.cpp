#include "core/graphics/rendering_canvas.h"

namespace golias {
    void RenderingCanvas::Submit(const DrawCommand& command) {
        command_queue.push_back(command);
    }


    void RenderingCanvas::Draw(RenderingDevice* rd, const CameraData& camera) {

        for (const auto& command : command_queue) {
            
            
            rd->BindMaterial(command.material);

            auto shader = command.material->GetShader();
            
            shader->SetUniform("MODEL_MATRIX", command.modelMatrix);
            shader->SetUniform("VIEW_MATRIX", camera.viewMatrix);
            shader->SetUniform("PROJECTION_MATRIX", camera.projectionMatrix);

            rd->BindMesh(command.mesh);
            rd->DrawMesh(command.mesh);
        }
        command_queue.clear();
    }
}; // namespace golias
