#pragma once

#include "scene/3d/camera_component.h"
#include "scene/game_object.h"

namespace golias {

    class CameraComponent : public Component {
        COMPONENT(CameraComponent)
    public:
        CameraComponent() = default;

        virtual ~CameraComponent() = default;

        void Start() override;
        void Update(float deltaTime) override;

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float aspectRatio) const;
     

        void SetFOV(float newFov);
        float GetFOV() const;

        void SetNearPlane(float newNearPlane);
        float GetNearPlane() const;

        void SetFarPlane(float newFarPlane);
        float GetFarPlane() const;

    private:
        float fov        = 45.0f;
        float near_plane = 0.1f;
        float far_plane  = 1000.0f;

    };
}; // namespace golias
