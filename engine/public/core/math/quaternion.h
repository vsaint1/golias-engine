#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Vector3;
class Basis;
class Matrix4;

class Quaternion : public glm::quat {
public:
    using glm::quat::quat;

    Quaternion();
    Quaternion(float w, float x, float y, float z);
    Quaternion(const glm::quat& q);
    Quaternion(const glm::vec3& euler_angles);

    float& operator[](int index);
    const float& operator[](int index) const;

    float length() const;

    Quaternion normalized() const;
    void normalize();

    Quaternion conjugate() const;
    Quaternion inverse() const;

    float dot(const Quaternion& other) const;

    Quaternion slerp(const Quaternion& other, float t) const;

    Vector3 xform(const Vector3& v) const;
    Vector3 to_euler() const;
    Matrix4 to_mat4() const;
    Basis to_basis() const;

    static Quaternion from_axis_angle(const Vector3& axis, float angle);
    static Quaternion from_euler(float pitch, float yaw, float roll);
    static Quaternion from_euler(const Vector3& euler);
    static Quaternion look_at(const Vector3& direction, const Vector3& up);

    static const Quaternion IDENTITY;
};


