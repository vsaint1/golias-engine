#include "core/math/vector3.h"
#include <cmath>

// ============================================================================
// Vector3 Implementation
// ============================================================================

Vector3::Vector3() : glm::vec3(0.0f, 0.0f, 0.0f) {}
Vector3::Vector3(float x, float y, float z) : glm::vec3(x, y, z) {}
Vector3::Vector3(float v) : glm::vec3(v, v, v) {}
Vector3::Vector3(const glm::vec3& v) : glm::vec3(v) {}

float& Vector3::operator[](int index) { return glm::vec3::operator[](index); }
const float& Vector3::operator[](int index) const { return glm::vec3::operator[](index); }

float Vector3::length() const {
    return glm::length(static_cast<const glm::vec3&>(*this));
}

Vector3 Vector3::normalized() const {
    return glm::normalize(static_cast<const glm::vec3&>(*this));
}

void Vector3::normalize() {
    *this = glm::normalize(static_cast<const glm::vec3&>(*this));
}

float Vector3::dot(const Vector3& other) const {
    return glm::dot(static_cast<const glm::vec3&>(*this), static_cast<const glm::vec3&>(other));
}

Vector3 Vector3::cross(const Vector3& other) const {
    return glm::cross(static_cast<const glm::vec3&>(*this), static_cast<const glm::vec3&>(other));
}

float Vector3::distance_to(const Vector3& other) const {
    return glm::distance(static_cast<const glm::vec3&>(*this), static_cast<const glm::vec3&>(other));
}


Vector3 Vector3::lerp(const Vector3& other, float t) const {
    return Vector3(x + (other.x - x) * t,
                  y + (other.y - y) * t,
                  z + (other.z - z) * t);
}

Vector3 Vector3::reflect(const Vector3& normal) const {
    return glm::reflect(static_cast<const glm::vec3&>(*this), static_cast<const glm::vec3&>(normal));
}

Vector3 Vector3::refract(const Vector3& normal, float eta) const {
    return glm::refract(static_cast<const glm::vec3&>(*this), static_cast<const glm::vec3&>(normal), eta);
}

Vector3 Vector3::abs() const {
    return Vector3(std::abs(x), std::abs(y), std::abs(z));
}

Vector3 Vector3::sign() const {
    return Vector3(x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f),
                  y > 0.0f ? 1.0f : (y < 0.0f ? -1.0f : 0.0f),
                  z > 0.0f ? 1.0f : (z < 0.0f ? -1.0f : 0.0f));
}

Vector3 Vector3::floor() const {
    return Vector3(std::floor(x), std::floor(y), std::floor(z));
}

Vector3 Vector3::ceil() const {
    return Vector3(std::ceil(x), std::ceil(y), std::ceil(z));
}

Vector3 Vector3::round() const {
    return Vector3(std::round(x), std::round(y), std::round(z));
}

bool Vector3::is_zero_approx() const {
    return glm::length(static_cast<const glm::vec3&>(*this)) < 1e-6f;
}


const Vector3 Vector3::ZERO    = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE     = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::LEFT    = Vector3(-1.0f, 0.0f, 0.0f);
const Vector3 Vector3::RIGHT   = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UP      = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::DOWN    = Vector3(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::BACK    = Vector3(0.0f, 0.0f, 1.0f);

// ============================================================================
// Vector3i Implementation
// ============================================================================

Vector3i::Vector3i() : glm::ivec3(0, 0, 0) {}
Vector3i::Vector3i(int x, int y, int z) : glm::ivec3(x, y, z) {}
Vector3i::Vector3i(int v) : glm::ivec3(v, v, v) {}
Vector3i::Vector3i(const glm::ivec3& v) : glm::ivec3(v) {}

int& Vector3i::operator[](int index) { return glm::ivec3::operator[](index); }
const int& Vector3i::operator[](int index) const { return glm::ivec3::operator[](index); }


float Vector3i::length() const {
    return glm::length(static_cast<const glm::vec3&>(*this));
}

Vector3i Vector3i::abs() const {
    return Vector3i(std::abs(x), std::abs(y), std::abs(z));
}

Vector3i Vector3i::sign() const {
    return Vector3i(x > 0 ? 1 : (x < 0 ? -1 : 0),
                   y > 0 ? 1 : (y < 0 ? -1 : 0),
                   z > 0 ? 1 : (z < 0 ? -1 : 0));
}

const Vector3i Vector3i::ZERO    = Vector3i(0, 0, 0);
const Vector3i Vector3i::ONE     = Vector3i(1, 1, 1);
const Vector3i Vector3i::LEFT    = Vector3i(-1, 0, 0);
const Vector3i Vector3i::RIGHT   = Vector3i(1, 0, 0);
const Vector3i Vector3i::UP      = Vector3i(0, 1, 0);
const Vector3i Vector3i::DOWN    = Vector3i(0, -1, 0);
const Vector3i Vector3i::FORWARD = Vector3i(0, 0, -1);
const Vector3i Vector3i::BACK    = Vector3i(0, 0, 1);

