#include "core/math/vector2.h"
#include <cmath>

// ============================================================================
// Vector2 Implementation
// ============================================================================

Vector2::Vector2() : glm::vec2(0.0f, 0.0f) {}
Vector2::Vector2(float x, float y) : glm::vec2(x, y) {}
Vector2::Vector2(float v) : glm::vec2(v, v) {}
Vector2::Vector2(const glm::vec2& v) : glm::vec2(v) {}

float& Vector2::operator[](int index) { return glm::vec2::operator[](index); }
const float& Vector2::operator[](int index) const { return glm::vec2::operator[](index); }

float Vector2::length() const {
    return glm::length(static_cast<const glm::vec2&>(*this));
}

Vector2 Vector2::normalized() const {
    return glm::normalize(static_cast<const glm::vec2&>(*this));
}

void Vector2::normalize() {
    *this = glm::normalize(static_cast<const glm::vec2&>(*this));
}

float Vector2::dot(const Vector2& other) const {
    return glm::dot(static_cast<const glm::vec2&>(*this), static_cast<const glm::vec2&>(other));
}

float Vector2::distance_to(const Vector2& other) const {
    return glm::distance(static_cast<const glm::vec2&>(*this), static_cast<const glm::vec2&>(other));
}


Vector2 Vector2::lerp(const Vector2& other, float t) const {
    return Vector2(x + (other.x - x) * t, y + (other.y - y) * t);
}

Vector2 Vector2::reflect(const Vector2& normal) const {
    return glm::reflect(static_cast<const glm::vec2&>(*this), static_cast<const glm::vec2&>(normal));
}

Vector2 Vector2::refract(const Vector2& normal, float eta) const {
    return glm::refract(static_cast<const glm::vec2&>(*this), static_cast<const glm::vec2&>(normal), eta);
}

Vector2 Vector2::abs() const {
    return Vector2(std::abs(x), std::abs(y));
}

Vector2 Vector2::sign() const {
    return Vector2(x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f),
                  y > 0.0f ? 1.0f : (y < 0.0f ? -1.0f : 0.0f));
}

Vector2 Vector2::floor() const {
    return Vector2(std::floor(x), std::floor(y));
}

Vector2 Vector2::ceil() const {
    return Vector2(std::ceil(x), std::ceil(y));
}

Vector2 Vector2::round() const {
    return Vector2(std::round(x), std::round(y));
}

float Vector2::angle() const {
    return std::atan2(y, x);
}

float Vector2::angle_to(const Vector2& other) const {
    return std::atan2(other.y - y, other.x - x);
}

Vector2 Vector2::rotated(float angle) const {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Vector2(x * c - y * s, x * s + y * c);
}

bool Vector2::is_zero_approx() const {
    return glm::length(static_cast<const glm::vec2&>(*this)) < 1e-6f;
}

const Vector2 Vector2::ZERO  = Vector2(0.0f, 0.0f);
const Vector2 Vector2::ONE   = Vector2(1.0f, 1.0f);
const Vector2 Vector2::LEFT  = Vector2(-1.0f, 0.0f);
const Vector2 Vector2::RIGHT = Vector2(1.0f, 0.0f);
const Vector2 Vector2::UP    = Vector2(0.0f, -1.0f);
const Vector2 Vector2::DOWN  = Vector2(0.0f, 1.0f);

// ============================================================================
// Vector2i Implementation
// ============================================================================

Vector2i::Vector2i() : glm::ivec2(0, 0) {}
Vector2i::Vector2i(int x, int y) : glm::ivec2(x, y) {}
Vector2i::Vector2i(int v) : glm::ivec2(v, v) {}
Vector2i::Vector2i(const glm::ivec2& v) : glm::ivec2(v) {}

int& Vector2i::operator[](int index) { return glm::ivec2::operator[](index); }
const int& Vector2i::operator[](int index) const { return glm::ivec2::operator[](index); }


float Vector2i::length() const {
    return glm::length(static_cast<const glm::vec2&>(*this));
}

Vector2i Vector2i::abs() const {
    return Vector2i(std::abs(x), std::abs(y));
}

Vector2i Vector2i::sign() const {
    return Vector2i(x > 0 ? 1 : (x < 0 ? -1 : 0),
                   y > 0 ? 1 : (y < 0 ? -1 : 0));
}

const Vector2i Vector2i::ZERO  = Vector2i(0, 0);
const Vector2i Vector2i::ONE   = Vector2i(1, 1);
const Vector2i Vector2i::LEFT  = Vector2i(-1, 0);
const Vector2i Vector2i::RIGHT = Vector2i(1, 0);
const Vector2i Vector2i::UP    = Vector2i(0, -1);
const Vector2i Vector2i::DOWN  = Vector2i(0, 1);

