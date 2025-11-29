#pragma once

#include <glm/glm.hpp>

class Vector3 : public glm::vec3 {
public:
    using glm::vec3::vec3;

    Vector3();
    Vector3(float x, float y, float z);
    Vector3(float v);
    Vector3(const glm::vec3& v);

    float& operator[](int index);
    const float& operator[](int index) const;

    float length() const;

    Vector3 normalized() const;
    void normalize();

    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;

    float distance_to(const Vector3& other) const;

    Vector3 lerp(const Vector3& other, float t) const;

    Vector3 reflect(const Vector3& normal) const;
    Vector3 refract(const Vector3& normal, float eta) const;

    Vector3 abs() const;
    Vector3 sign() const;

    Vector3 floor() const;
    Vector3 ceil() const;
    Vector3 round() const;

    bool is_zero_approx() const;

    static const Vector3 ZERO;
    static const Vector3 ONE;
    static const Vector3 LEFT;
    static const Vector3 RIGHT;
    static const Vector3 UP;
    static const Vector3 DOWN;
    static const Vector3 FORWARD;
    static const Vector3 BACK;
};

class Vector3i : public glm::ivec3 {
public:
    using glm::ivec3::ivec3;

    Vector3i();
    Vector3i(int x, int y, int z);
    Vector3i(int v);
    Vector3i(const glm::ivec3& v);

    int& operator[](int index);
    const int& operator[](int index) const;

    float length() const;

    Vector3i abs() const;
    Vector3i sign() const;

    static const Vector3i ZERO;
    static const Vector3i ONE;
    static const Vector3i LEFT;
    static const Vector3i RIGHT;
    static const Vector3i UP;
    static const Vector3i DOWN;
    static const Vector3i FORWARD;
    static const Vector3i BACK;
};

using Vec3 = Vector3;
using Vec3i = Vector3i;

