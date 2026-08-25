#pragma once

#include "component.h"

namespace golias {

    class CameraComponent : public Component {

        COMPONENT(CameraComponent)
    public:
        CameraComponent() = default;
        CameraComponent(float fov, float aspectRatio, float nearPlane, float farPlane);

        void Update(float deltaTime) override;

        glm::mat4 GetViewMatrix() const;

        glm::mat4 GetProjectionMatrix() const;

        float GetFOV() const;
        void SetFOV(float fov);

        float GetAspectRatio() const;
        void SetAspectRatio(float aspectRatio);

        float GetNearPlane() const;
        void SetNearPlane(float nearPlane);

        float GetFarPlane() const;
        void SetFarPlane(float farPlane);

    private:
        float mFOV         = 45.0f;
        float mAspectRatio = 1.0f;
        float mNearPlane   = 0.1f;
        float mFarPlane    = 100.0f;
    };
} // namespace golias
