#include "core/graphics/rendering_canvas.h"

namespace golias {
    void RenderingCanvas::Submit(const DrawCommand& command) {
        command_queue.push_back(command);
    }


    void RenderingCanvas::Draw(RenderingDevice* rd, const CameraData& camera) {
        for (const auto& command : command_queue) {

            rd->BindMaterial(command.material);
            command.material->SetParameter("MODEL_MATRIX", command.modelMatrix);
            command.material->SetParameter("VIEW_MATRIX", camera.viewMatrix);
            command.material->SetParameter("PROJECTION_MATRIX", camera.projectionMatrix);

            rd->BindMesh(command.mesh);

            rd->DrawMesh(command.mesh);
        }

        command_queue.clear();
    }
}; // namespace golias
