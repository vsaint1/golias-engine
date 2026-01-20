#include "scene/3d/character_controller_component.h"

#include "core/engine.h"
#include <btBulletDynamicsCommon.h>

#include <glm/gtc/matrix_transform.hpp>

namespace golias {
    void CharacterControllerComponent::Start() {
        const auto& pos     = GetOwner()->GetWorldPosition();
        characterController = std::make_unique<KinematicCharacterController>(1.6f, 0.4f, pos);
        characterController->SetPosition(pos);
        GetOwner()->SetPosition(characterController->GetPosition());
    }

    void CharacterControllerComponent::Update(float deltaTime) {
        if (!characterController) {
            return;
        }


        GetOwner()->SetPosition(characterController->GetPosition());
    }

    void CharacterControllerComponent::Move(const glm::vec3& motion) {
        if (!characterController) {
            return;
        }


        characterController->SetWalkDirection(motion);
    }

    void CharacterControllerComponent::Jump(const glm::vec3& force) {
        if (!characterController) {
            return;
        }

        characterController->Jump(force);
    }

    void CharacterControllerComponent::LoadProperties(const nlohmann::json& json) {
    }


    float CharacterControllerComponent::GetHeight() const {
        return controllerHeight;
    }

    void CharacterControllerComponent::SetHeight(float h) {
        controllerHeight = h;
    }

    float CharacterControllerComponent::GetRadius() const {
        return controllerRadius;
    }

    void CharacterControllerComponent::SetRadius(float r) {
        controllerRadius = r;
    }

    bool CharacterControllerComponent::IsGrounded() const {
        if (characterController) {
            return characterController->OnGround();
        }


        return false; /// ??? (UB)
    }

    glm::vec3 CharacterControllerComponent::GetMotion() const {
        return controllerMotion;
    }

    KinematicCharacterController* CharacterControllerComponent::GetKinematicController() const {
        return characterController.get();
    }


} // namespace golias
