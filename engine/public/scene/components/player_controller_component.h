#pragma once
#include "component.h"

namespace golias {

    class KinematicCharacterController;

    class PlayerControllerComponent : public Component {

        COMPONENT(PlayerControllerComponent)
    public:
        PlayerControllerComponent() = default;

        void Start() override;

        void Update(float deltaTime) override;

        float GetMoveSpeed() const;
        void SetMoveSpeed(float speed);

        float GetSensitivity() const;
        void SetSensitivity(float sensitivity);

        KinematicCharacterController* GetCharacterController() const;

    private:
        float mMoveSpeed   = 10.0f;
        float mSensitivity = 0.1f;

        KinematicCharacterController* mCharacterController = nullptr;
    };
} // namespace golias
