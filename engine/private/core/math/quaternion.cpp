#include "core/math/quaternion.h"
#include "core/math/vector3.h"
#include "core/math/basis.h"
#include "core/math/matrix4.h"

// ============================================================================
// Quaternion Implementation
// ============================================================================

Quaternion::Quaternion() : glm::quat(1.0f, 0.0f, 0.0f, 0.0f) {}
Quaternion::Quaternion(float w, float x, float y, float z) : glm::quat(w, x, y, z) {}
Quaternion::Quaternion(const glm::quat& q) : glm::quat(q) {}
Quaternion::Quaternion(const glm::vec3& euler_angles) : glm::quat(euler_angles) {}

float& Quaternion::operator[](int index) { return glm::quat::operator[](index); }
const float& Quaternion::operator[](int index) const { return glm::quat::operator[](index); }

float Quaternion::length() const {
    return glm::length(static_cast<const glm::quat&>(*this));
}

Quaternion Quaternion::normalized() const {
    return glm::normalize(static_cast<const glm::quat&>(*this));
}

void Quaternion::normalize() {
    *this = glm::normalize(static_cast<const glm::quat&>(*this));
}

Quaternion Quaternion::conjugate() const {
    return glm::conjugate(static_cast<const glm::quat&>(*this));
}

Quaternion Quaternion::inverse() const {
    return glm::inverse(static_cast<const glm::quat&>(*this));
}

float Quaternion::dot(const Quaternion& other) const {
    return glm::dot(static_cast<const glm::quat&>(*this), static_cast<const glm::quat&>(other));
}

Quaternion Quaternion::slerp(const Quaternion& other, float t) const {
    return glm::slerp(static_cast<const glm::quat&>(*this), static_cast<const glm::quat&>(other), t);
}

Vector3 Quaternion::xform(const Vector3& v) const {
    return (*this) * v;
}

Vector3 Quaternion::to_euler() const {
    return glm::eulerAngles(*this);
}

Matrix4 Quaternion::to_mat4() const {
    return glm::toMat4(*this);
}

Basis Quaternion::to_basis() const {
    return glm::toMat3(*this);
}

Quaternion Quaternion::from_axis_angle(const Vector3& axis, float angle) {
    return glm::angleAxis(angle, axis);
}

Quaternion Quaternion::from_euler(float pitch, float yaw, float roll) {
    return Quaternion(Vector3(pitch, yaw, roll));
}

Quaternion Quaternion::from_euler(const Vector3& euler) {
    return Quaternion(euler);
}

Quaternion Quaternion::look_at(const Vector3& direction, const Vector3& up) {
    return glm::quatLookAt(direction, up);
}

const Quaternion Quaternion::IDENTITY = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);

