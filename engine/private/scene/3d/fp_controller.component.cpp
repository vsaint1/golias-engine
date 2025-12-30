#include "core/engine.h"
#include "scene/3d/fp_controller_component.h"

#include <glm/gtc/matrix_transform.hpp>

namespace golias {

    void FirstPersonControllerComponent::Start() {
        characterController = std::make_unique<KinematicCharacterController>(0.4f, 1.2f);
    }

    void FirstPersonControllerComponent::Update(float deltaTime) {
        auto& input   = Engine::GetInstance().GetInputManager();
        auto rotation = GetOwner()->GetRotation();

        const auto mouse_delta = input.GetMouseDelta();

        if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f) {

            yaw += -mouse_delta.x * sensitivity;
            pitch += -mouse_delta.y * sensitivity;

            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            glm::quat rotY = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
            glm::quat rotX = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));

            rotation = glm::normalize(rotY * rotX);
            GetOwner()->SetRotation(rotation);
        }

        glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

        auto position = GetOwner()->GetPosition();

        glm::vec3 velocity(0.0f);

        // Left/Right movement
        if (input.IsKeyPressed(SDLK_A)) {
            velocity -= right;
        }

        if (input.IsKeyPressed(SDLK_D)) {
            velocity += right;
        }

        // Vertical movement
        if (input.IsKeyPressed(SDLK_W)) {
            velocity += front;
        }
        if (input.IsKeyPressed(SDLK_S)) {
            velocity -= front;
        }

        if (input.IsKeyPressed(SDLK_SPACE)) {
            characterController->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
        }

        if (glm::dot(velocity, velocity) > 0.0f) {
            velocity = glm::normalize(velocity) * speed * deltaTime;
        }

        characterController->Move(velocity);

        GetOwner()->SetPosition(characterController->GetPosition());
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
