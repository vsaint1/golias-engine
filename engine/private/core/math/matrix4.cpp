#include "core/math/matrix4.h"
#include "core/math/vector3.h"
#include "core/math/quaternion.h"
#include "core/math/basis.h"

// ============================================================================
// Matrix4 Implementation
// ============================================================================

Matrix4::Matrix4() : glm::mat4(1.0f) {}
Matrix4::Matrix4(float diagonal) : glm::mat4(diagonal) {}
Matrix4::Matrix4(const glm::mat4& m) : glm::mat4(m) {}

glm::vec4& Matrix4::operator[](int index) { return glm::mat4::operator[](index); }
const glm::vec4& Matrix4::operator[](int index) const { return glm::mat4::operator[](index); }

Matrix4 Matrix4::transposed() const {
    return glm::transpose(*this);
}

void Matrix4::transpose() {
    *this = glm::transpose(*this);
}

Matrix4 Matrix4::inverse() const {
    return glm::inverse(*this);
}

void Matrix4::invert() {
    *this = glm::inverse(*this);
}

float Matrix4::determinant() const {
    return glm::determinant(*this);
}

Vector3 Matrix4::xform(const Vector3& v) const {
    glm::vec4 result = (*this) * glm::vec4(v.x, v.y, v.z, 1.0f);
    return Vector3(result.x, result.y, result.z) / result.w;
}

Vector3 Matrix4::xform_inv(const Vector3& v) const {
    Matrix4 inv = glm::inverse(*this);
    return inv.xform(v);
}

Vector3 Matrix4::xform_normal(const Vector3& normal) const {
    glm::vec4 result = (*this) * glm::vec4(normal.x, normal.y, normal.z, 0.0f);
    return Vector3(result.x, result.y, result.z);
}

Basis Matrix4::get_basis() const {
    return Basis(glm::mat3(*this));
}

Vector3 Matrix4::get_translation() const {
    return Vector3((*this)[3].x, (*this)[3].y, (*this)[3].z);
}

Quaternion Matrix4::to_quaternion() const {
    return glm::quat_cast(*this);
}

Matrix4 Matrix4::translate(const Vector3& translation) {
    return glm::translate(Matrix4(1.0f), translation);
}

Matrix4 Matrix4::rotate(float angle, const Vector3& axis) {
    return glm::rotate(Matrix4(1.0f), angle, axis);
}

Matrix4 Matrix4::scale(const Vector3& scale_vec) {
    return glm::scale(Matrix4(1.0f), scale_vec);
}

Matrix4 Matrix4::look_at(const Vector3& eye, const Vector3& center, const Vector3& up) {
    return glm::lookAt(eye, center, up);
}

Matrix4 Matrix4::perspective(float fov, float aspect, float near_plane, float far_plane) {
    return glm::perspective(fov, aspect, near_plane, far_plane);
}

Matrix4 Matrix4::ortho(float left, float right, float bottom, float top, float near_plane, float far_plane) {
    return glm::ortho(left, right, bottom, top, near_plane, far_plane);
}

Matrix4 Matrix4::from_quaternion(const Quaternion& q) {
    return glm::toMat4(q);
}

const Matrix4 Matrix4::IDENTITY = Matrix4(1.0f);

