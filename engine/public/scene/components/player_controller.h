#pragma once
#include "component.h"

namespace golias {

    class PlayerControllerComponent : public Component {

        COMPONENT(PlayerControllerComponent)
    public:
        void Update(float deltaTime) override;


    private:
        float mMoveSpeed   = 1.0f;
        float mSensitivity = 0.1f;
    };
} // namespace golias
