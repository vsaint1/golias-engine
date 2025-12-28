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
            shader->SetUniform("VIEW_POSITION", camera.position);
            shader->SetUniform("LIGHT_POSITION", glm::vec3(50.0f, 100.0f, 80.0f));
            shader->SetUniform("LIGHT_COLOR", glm::vec3(1.0f, 0.95f, 0.9f)); 
            shader->SetUniform("AMBIENT_STRENGTH", 0.25f); 
            shader->SetUniform("SPECULAR_STRENGTH", 0.4f);
            shader->SetUniform("SHININESS", 32.0f);

            rd->BindMesh(command.mesh);
            rd->DrawMesh(command.mesh);
        }

        
        command_queue.clear();
    }
}; // namespace golias
