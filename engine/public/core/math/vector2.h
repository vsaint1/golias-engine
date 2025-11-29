#pragma once

#include <glm/glm.hpp>

class Vector2 : public glm::vec2 {
public:
    using glm::vec2::vec2;

    Vector2();
    Vector2(float x, float y);
    Vector2(float v);
    Vector2(const glm::vec2& v);

    float& operator[](int index);
    const float& operator[](int index) const;

    float length() const;

    Vector2 normalized() const;
    void normalize();

    float dot(const Vector2& other) const;
    float distance_to(const Vector2& other) const;

    Vector2 lerp(const Vector2& other, float t) const;

    Vector2 reflect(const Vector2& normal) const;
    Vector2 refract(const Vector2& normal, float eta) const;

    Vector2 abs() const;
    Vector2 sign() const;

    Vector2 floor() const;
    Vector2 ceil() const;
    Vector2 round() const;

    float angle() const;
    float angle_to(const Vector2& other) const;

    Vector2 rotated(float angle) const;

    bool is_zero_approx() const;

    static const Vector2 ZERO;
    static const Vector2 ONE;
    static const Vector2 LEFT;
    static const Vector2 RIGHT;
    static const Vector2 UP;
    static const Vector2 DOWN;
};

class Vector2i : public glm::ivec2 {
public:
    using glm::ivec2::ivec2;

    Vector2i();
    Vector2i(int x, int y);
    Vector2i(int v);
    Vector2i(const glm::ivec2& v);

    int& operator[](int index);
    const int& operator[](int index) const;

    float length() const;

    Vector2i abs() const;
    Vector2i sign() const;

    static const Vector2i ZERO;
    static const Vector2i ONE;
    static const Vector2i LEFT;
    static const Vector2i RIGHT;
    static const Vector2i UP;
    static const Vector2i DOWN;
};

// Aliases
using Vec2 = Vector2;
using Vec2i = Vector2i;

