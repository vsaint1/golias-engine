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

            // Scene properties
            shader->SetUniform("u_viewPosition", camera.position);
            shader->SetUniform("u_ambientStrength", 0.3f);
            shader->SetUniform("u_specularStrength", 0.5f);
            shader->SetUniform("u_shininess", 32.0f);

            // Directional lights
            shader->SetUniform("u_directionalLightCount", 1);
            shader->SetUniform("u_directionalLights[0].direction", glm::vec3(0.5f, -1.0f, 0.3f));
            shader->SetUniform("u_directionalLights[0].color", glm::vec3(1.0f, 1.0f, 1.0f));
            shader->SetUniform("u_directionalLights[0].intensity", 1.0f);

            if (command.mesh) {

                rd->BindMesh(command.mesh);

                rd->DrawMesh(command.mesh);

                rd->UnbindMesh(command.mesh);
                
            }
        }


        command_queue.clear();
    }
}; // namespace golias
