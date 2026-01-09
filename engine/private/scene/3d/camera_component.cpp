#include "scene/3d/camera_component.h"

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

namespace golias {

    void CameraComponent::Start() {
    }
    
    void CameraComponent::Update(float deltaTime) {
    }

    glm::mat4 CameraComponent::GetViewMatrix() const {

        glm::mat4 mat(1.0f);
        mat = glm::mat4_cast(GetOwner()->GetRotation());
        mat = glm::translate(mat, GetOwner()->GetPosition());

        mat[3] = glm::vec4(GetOwner()->GetPosition(), 1.0f);

        if (GetOwner()->GetParent()) {
            mat = GetOwner()->GetParent()->GetWorldTransform() * mat;
        }

        mat = glm::inverse(mat);

        return mat;
    }

    glm::mat4 CameraComponent::GetProjectionMatrix(float aspectRatio) const {

        return glm::perspective(glm::radians(fov), aspectRatio, near_plane, far_plane);
    }


    float CameraComponent::GetFOV() const {
        return fov;
    }

    void CameraComponent::SetFOV(float newFov) {
        fov = newFov;
    }

    float CameraComponent::GetNearPlane() const {
        return near_plane;
    }

    void CameraComponent::SetNearPlane(float newNearPlane) {
        near_plane = newNearPlane;
    }

    float CameraComponent::GetFarPlane() const {
        return far_plane;
    }

    void CameraComponent::SetFarPlane(float newFarPlane) {
        far_plane = newFarPlane;
    }

}; // namespace golias
