#include "core/engine.h"
#include "scene/3d/fp_controller_component.h"

#include <glm/gtc/matrix_transform.hpp>

namespace golias {

    void FirstPersonControllerComponent::Start() {
       
    }

    void FirstPersonControllerComponent::Update(float deltaTime) {
        auto& input   = Engine::GetInstance().GetInputManager();
        auto rotation = GetOwner()->GetRotation();


        if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            const auto mouse_delta = input.GetMouseDelta();


            // rot around Y axis
            float yAngle   = -mouse_delta.x * sensitivity * deltaTime;
            glm::quat yRot = glm::angleAxis(yAngle, glm::vec3(0.0f, 1.0f, 0.0f));

            // rot around X axis
            float xAngle    = -mouse_delta.y * sensitivity * deltaTime;
            glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            glm::quat xRot  = glm::angleAxis(xAngle, right);

            glm::quat deltaRot = yRot * xRot;
            rotation           = glm::normalize(deltaRot * rotation);

            GetOwner()->SetRotation(rotation);
        }

        glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

        auto position = GetOwner()->GetPosition();

        // Left/Right movement
        if (input.IsKeyPressed(SDLK_A)) {
            position -= right * speed * deltaTime;
        }

        if (input.IsKeyPressed(SDLK_D)) {
            position += right * speed * deltaTime;
        }

        // Vertical movement
        if (input.IsKeyPressed(SDLK_W)) {
            position += front * speed * deltaTime;
        }
        if (input.IsKeyPressed(SDLK_S)) {
            position -= front * speed * deltaTime;
        }


        GetOwner()->SetPosition(position);
    }

    float FirstPersonControllerComponent::GetSensitivity() const {
        return sensitivity;
    }

    void FirstPersonControllerComponent::SetSensitivity(float value) {
        sensitivity = value;
    }

    float FirstPersonControllerComponent::GetSpeed() const {
        return speed;
    }

    void FirstPersonControllerComponent::SetSpeed(float value) {
        speed = value;
    }
} // namespace golias
