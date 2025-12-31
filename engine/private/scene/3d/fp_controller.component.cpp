#include "core/engine.h"
#include "scene/3d/fp_controller_component.h"

#include <glm/gtc/matrix_transform.hpp>

namespace golias {

    void FirstPersonControllerComponent::Start() {
        const auto& pos     = GetOwner()->GetWorldPosition();
        characterController = std::make_unique<KinematicCharacterController>(1.6f, 0.4f, pos);
        characterController->SetPosition(pos);
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

        glm::quat yawRot = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));

        glm::vec3 front = yawRot * glm::vec3(0, 0, -1);
        glm::vec3 right = yawRot * glm::vec3(1, 0, 0);

        front.y = 0.0f;
        right.y = 0.0f;

        front = glm::normalize(front);
        right = glm::normalize(right);

        glm::vec3 direction(0.0f);

        if (input.IsKeyPressed(SDLK_A)) {
            direction -= right;
        }
        if (input.IsKeyPressed(SDLK_D)) {
            direction += right;
        }
        if (input.IsKeyPressed(SDLK_W)) {
            direction += front;
        }
        if (input.IsKeyPressed(SDLK_S)) {
            direction -= front;
        }


        if (input.IsKeyPressed(SDLK_SPACE)) {
            characterController->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
        }

        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction) * speed * deltaTime;
        }

        characterController->Move(direction);

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

    KinematicCharacterController* FirstPersonControllerComponent::GetCharacterController() const {
        return characterController.get();
    }
} // namespace golias
