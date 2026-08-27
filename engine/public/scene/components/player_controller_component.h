#pragma once
#include "component.h"
#include "physics/collision.h"

namespace golias {

    class KinematicCharacterController;

    class PlayerControllerComponent : public Component, public ContactListener {

        COMPONENT(PlayerControllerComponent)
    public:
        PlayerControllerComponent() = default;
        ~PlayerControllerComponent() override;

        bool LoadProperties(const Json& properties) override;

        void Start() override;

        void Update(float deltaTime) override;

        void OnCollisionEnter(const Collision& collision) override;
        void OnCollisionExit(const Collision& collision) override;

        float GetMoveSpeed() const;
        void SetMoveSpeed(float speed);

        float GetSensitivity() const;
        void SetSensitivity(float sensitivity);

        KinematicCharacterController* GetCharacterController() const;

        bool OnGround() const;

        void Jump(const glm::vec3& direction);

    private:
        float mMoveSpeed   = 10.0f;
        float mSensitivity = 0.1f;

        float mRadius = 0.4f;
        float mHeight = 1.2f;

        KinematicCharacterController* mCharacterController = nullptr;
    };
} // namespace golias
