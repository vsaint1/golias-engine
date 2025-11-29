#include "core/math/vector4.h"
#include <cmath>

// ============================================================================
// Vector4 Implementation
// ============================================================================

Vector4::Vector4() : glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) {}
Vector4::Vector4(float x, float y, float z, float w) : glm::vec4(x, y, z, w) {}
Vector4::Vector4(float v) : glm::vec4(v, v, v, v) {}
Vector4::Vector4(const glm::vec4& v) : glm::vec4(v) {}

float& Vector4::operator[](int index) { return glm::vec4::operator[](index); }
const float& Vector4::operator[](int index) const { return glm::vec4::operator[](index); }

float Vector4::length() const {
    return glm::length(static_cast<const glm::vec4&>(*this));
}

Vector4 Vector4::normalized() const {
    return glm::normalize(static_cast<const glm::vec4&>(*this));
}

void Vector4::normalize() {
    *this = glm::normalize(static_cast<const glm::vec4&>(*this));
}

float Vector4::dot(const Vector4& other) const {
    return glm::dot(static_cast<const glm::vec4&>(*this), static_cast<const glm::vec4&>(other));
}

float Vector4::distance_to(const Vector4& other) const {
    return glm::distance(static_cast<const glm::vec4&>(*this), static_cast<const glm::vec4&>(other));
}

Vector4 Vector4::lerp(const Vector4& other, float t) const {
    return Vector4(x + (other.x - x) * t,
                  y + (other.y - y) * t,
                  z + (other.z - z) * t,
                  w + (other.w - w) * t);
}

Vector4 Vector4::abs() const {
    return Vector4(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

Vector4 Vector4::floor() const {
    return Vector4(std::floor(x), std::floor(y), std::floor(z), std::floor(w));
}

Vector4 Vector4::ceil() const {
    return Vector4(std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w));
}

Vector4 Vector4::round() const {
    return Vector4(std::round(x), std::round(y), std::round(z), std::round(w));
}

bool Vector4::is_zero_approx() const {
    return length() < 1e-12f;
}


const Vector4 Vector4::ZERO = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::ONE  = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

// ============================================================================
// Vector4i Implementation
// ============================================================================

Vector4i::Vector4i() : glm::ivec4(0, 0, 0, 0) {}
Vector4i::Vector4i(int x, int y, int z, int w) : glm::ivec4(x, y, z, w) {}
Vector4i::Vector4i(int v) : glm::ivec4(v, v, v, v) {}
Vector4i::Vector4i(const glm::ivec4& v) : glm::ivec4(v) {}

int& Vector4i::operator[](int index) { return glm::ivec4::operator[](index); }
const int& Vector4i::operator[](int index) const { return glm::ivec4::operator[](index); }


float Vector4i::length() const {
    return glm::length(static_cast<const glm::vec4&>(*this));
}

Vector4i Vector4i::abs() const {
    return Vector4i(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

const Vector4i Vector4i::ZERO = Vector4i(0, 0, 0, 0);
const Vector4i Vector4i::ONE  = Vector4i(1, 1, 1, 1);

