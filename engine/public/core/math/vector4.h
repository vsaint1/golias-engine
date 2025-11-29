#pragma once

#include <glm/glm.hpp>

class Vector4 : public glm::vec4 {
public:
    using glm::vec4::vec4;

    Vector4();
    Vector4(float x, float y, float z, float w);
    Vector4(float v);
    Vector4(const glm::vec4& v);

    float& operator[](int index);
    const float& operator[](int index) const;

    float length() const;

    Vector4 normalized() const;
    void normalize();

    float dot(const Vector4& other) const;

    float distance_to(const Vector4& other) const;

    Vector4 lerp(const Vector4& other, float t) const;

    Vector4 abs() const;
    Vector4 floor() const;
    Vector4 ceil() const;
    Vector4 round() const;

    bool is_zero_approx() const;

    static const Vector4 ZERO;
    static const Vector4 ONE;
};

class Vector4i : public glm::ivec4 {
public:
    using glm::ivec4::ivec4;

    Vector4i();
    Vector4i(int x, int y, int z, int w);
    Vector4i(int v);
    Vector4i(const glm::ivec4& v);

    int& operator[](int index);
    const int& operator[](int index) const;

    float length() const;

    Vector4i abs() const;

    static const Vector4i ZERO;
    static const Vector4i ONE;
};


