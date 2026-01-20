#pragma once
#include "physics/3d/kinematic_body.h"
#include "scene/component.h"

namespace golias {
    class CharacterControllerComponent : public Component {
        COMPONENT(CharacterControllerComponent)
    public:
        CharacterControllerComponent(float height = 1.6f, float radius = 0.4f) : controllerHeight(height), controllerRadius(radius) {
        }

        void Start() override;

        void Update(float deltaTime) override;

        void Move(const glm::vec3& velocity);

        void Jump(const glm::vec3& force);

        void SetHeight(float h) ;
        float GetHeight() const ;

        void SetRadius(float r);
        float GetRadius() const ;

        bool IsGrounded() const;

        glm::vec3 GetMotion() const;

        KinematicCharacterController* GetKinematicController() const;

        void LoadProperties(const nlohmann::json& json) override;

    private:
        std::unique_ptr<KinematicCharacterController> characterController = nullptr;

        float controllerHeight;
        float controllerRadius;

        glm::vec3 controllerMotion{0.0f};
    };
} // namespace golias
