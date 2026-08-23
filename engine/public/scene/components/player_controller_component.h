#pragma once
#include "component.h"

namespace golias {

    class PlayerControllerComponent : public Component {

        COMPONENT(PlayerControllerComponent)
    public:
        void Update(float deltaTime) override;

        float GetMoveSpeed() const;
        void SetMoveSpeed(float speed);

        float GetSensitivity() const;
        void SetSensitivity(float sensitivity);

    private:
        float mMoveSpeed   = 1.0f;
        float mSensitivity = 0.1f;
    };
} // namespace golias
