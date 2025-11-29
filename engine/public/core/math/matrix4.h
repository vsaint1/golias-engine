#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Vector3;
class Quaternion;
class Basis;

class Matrix4 : public glm::mat4 {
public:
    using glm::mat4::mat4;

    Matrix4();
    Matrix4(float diagonal);
    Matrix4(const glm::mat4& m);

    glm::vec4& operator[](int index);
    const glm::vec4& operator[](int index) const;

    Matrix4 transposed() const;
    void transpose();

    Matrix4 inverse() const;
    void invert();

    float determinant() const;

    Vector3 xform(const Vector3& v) const;
    Vector3 xform_inv(const Vector3& v) const;
    Vector3 xform_normal(const Vector3& normal) const;

    Basis get_basis() const;
    Vector3 get_translation() const;
    Quaternion to_quaternion() const;

    static Matrix4 translate(const Vector3& translation);
    static Matrix4 rotate(float angle, const Vector3& axis);
    static Matrix4 scale(const Vector3& scale_vec);
    static Matrix4 look_at(const Vector3& eye, const Vector3& center, const Vector3& up);
    static Matrix4 perspective(float fov, float aspect, float near_plane, float far_plane);
    static Matrix4 ortho(float left, float right, float bottom, float top, float near_plane, float far_plane);
    static Matrix4 from_quaternion(const Quaternion& q);

    static const Matrix4 IDENTITY;
};


