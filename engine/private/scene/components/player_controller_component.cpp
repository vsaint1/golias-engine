#include "scene/components/player_controller_component.h"

#include "core/engine.h"

namespace golias {

    float PlayerControllerComponent::GetMoveSpeed() const {
        return mMoveSpeed;
    }
    void PlayerControllerComponent::SetMoveSpeed(float speed) {
        mMoveSpeed = speed;
    }

    float PlayerControllerComponent::GetSensitivity() const {
        return mSensitivity;
    }

    void PlayerControllerComponent::SetSensitivity(float sensitivity) {
        mSensitivity = sensitivity;
    }


    void PlayerControllerComponent::Update(float deltaTime) {
        InputManager& inputManager = Engine::GetInstance().GetInputManager();

        glm::quat rotation = GetOwner()->GetRotation();

        if (inputManager.IsMouseButtonPressed(MouseButton::Right)) {
            glm::vec2 mouseDelta = inputManager.GetMouseDelta();

            float yaw   = -mouseDelta.x * mSensitivity;
            float pitch = -mouseDelta.y * mSensitivity;

            glm::quat yawRotation   = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat pitchRotation = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

            rotation = yawRotation * rotation * pitchRotation;
            GetOwner()->SetRotation(rotation);
        }

        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);

       
        glm::vec3 forward = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        forward.y          = 0.0f;
        forward            = glm::normalize(forward);

        glm::vec3 right = glm::vec3(rotationMatrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        right.y         = 0.0f;
        right           = glm::normalize(right);

        glm::vec3 position = GetOwner()->GetPosition();
        glm::vec3 movement(0.0f);

        if (inputManager.IsKeyPressed(KeyCode::W)) {
            movement += forward;
        }

        if (inputManager.IsKeyPressed(KeyCode::S)) {
            movement -= forward;
        }

        if (inputManager.IsKeyPressed(KeyCode::A)) {
            movement -= right;
        }

        if (inputManager.IsKeyPressed(KeyCode::D)) {
            movement += right;
        }

        if (glm::length2(movement) > 0.0f) {
            position += glm::normalize(movement) * mMoveSpeed * deltaTime;
        }

        GetOwner()->SetPosition(position);
    }


} // namespace golias
