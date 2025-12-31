#pragma once
#include "scene/component.h"
#include  "physics/3d/kinematic_body.h"

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

        KinematicCharacterController* GetCharacterController() const;

    private:
        float sensitivity = 0.4f;
        float speed       = 10.0f;

        float pitch = 0.0f;
        float yaw  = 0.0f;

        std::unique_ptr<KinematicCharacterController> characterController;
    };
} // namespace golias
