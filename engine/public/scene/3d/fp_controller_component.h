#pragma once
#include "scene/component.h"

namespace golias {

    class FirstPersonControllerComponent : public Component {
        COMPONENT(FirstPersonControllerComponent)
    public:
    
        void Start() override;
        void Update(float deltaTime) override;

        float GetSensitivity() const;
        void SetSensitivity(float value);

        float GetSpeed() const;
        void SetSpeed(float value);

    private:
        float sensitivity = 1.5f;
        float speed       = 5.0f;
    };
} // namespace golias
