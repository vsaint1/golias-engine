#include "scene/components/camera_component.h"

#include "scene/game_object.h"

namespace golias {

    CameraComponent::CameraComponent(float fov, float aspectRatio, float nearPlane, float farPlane)
        : mFOV(fov), mAspectRatio(aspectRatio), mNearPlane(nearPlane), mFarPlane(farPlane) {
    }

    void CameraComponent::Update(float deltaTime) {
    }

    glm::mat4 CameraComponent::GetViewMatrix() const {

        auto world = GetOwner()->GetWorldTransform();
        return glm::inverse(world);
    }

    glm::mat4 CameraComponent::GetProjectionMatrix() const {
        
        return glm::perspectiveLH_ZO(glm::radians(mFOV), mAspectRatio, mNearPlane, mFarPlane);
    }

    float CameraComponent::GetFOV() const {
        return mFOV;
    }

    void CameraComponent::SetFOV(float fov) {
        mFOV = fov;
    }

    float CameraComponent::GetAspectRatio() const {
        return mAspectRatio;
    }

    void CameraComponent::SetAspectRatio(float aspectRatio) {
        mAspectRatio = aspectRatio;
    }

    float CameraComponent::GetNearPlane() const {
        return mNearPlane;
    }

    void CameraComponent::SetNearPlane(float nearPlane) {
        mNearPlane = nearPlane;
    }

    float CameraComponent::GetFarPlane() const {
        return mFarPlane;
    }

    void CameraComponent::SetFarPlane(float farPlane) {
        mFarPlane = farPlane;
    }
} // namespace golias
