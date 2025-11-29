#include "core/math/basis.h"
#include "core/math/vector3.h"
#include "core/math/quaternion.h"

// ============================================================================
// Basis Implementation
// ============================================================================

Basis::Basis() : glm::mat3(1.0f) {}
Basis::Basis(float diagonal) : glm::mat3(diagonal) {}
Basis::Basis(const glm::mat3& m) : glm::mat3(m) {}

glm::vec3& Basis::operator[](int index) { return glm::mat3::operator[](index); }
const glm::vec3& Basis::operator[](int index) const { return glm::mat3::operator[](index); }

Basis Basis::transposed() const {
    return glm::transpose(*this);
}

void Basis::transpose() {
    *this = glm::transpose(*this);
}

Basis Basis::inverse() const {
    return glm::inverse(*this);
}

void Basis::invert() {
    *this = glm::inverse(*this);
}

float Basis::determinant() const {
    return glm::determinant(*this);
}

Vector3 Basis::xform(const Vector3& v) const {
    return (*this) * v;
}

Vector3 Basis::xform_inv(const Vector3& v) const {
    return glm::inverse(*this) * v;
}

Quaternion Basis::to_quaternion() const {
    return glm::quat_cast(*this);
}

Basis Basis::from_quaternion(const Quaternion& q) {
    return glm::toMat3(q);
}

Basis Basis::from_euler(const Vector3& euler) {
    return glm::toMat3(Quaternion(euler));
}

const Basis Basis::IDENTITY = Basis(1.0f);

