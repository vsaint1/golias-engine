#pragma once

#include <glm/glm.hpp>

class Vector3;
class Quaternion;

class Basis : public glm::mat3 {
public:
    using glm::mat3::mat3;

    Basis();
    Basis(float diagonal);
    Basis(const glm::mat3& m);

    glm::vec3& operator[](int index);
    const glm::vec3& operator[](int index) const;

    Basis transposed() const;
    void transpose();

    Basis inverse() const;
    void invert();

    float determinant() const;

    Vector3 xform(const Vector3& v) const;
    Vector3 xform_inv(const Vector3& v) const;

    Quaternion to_quaternion() const;

    static Basis from_quaternion(const Quaternion& q);
    static Basis from_euler(const Vector3& euler);

    static const Basis IDENTITY;
};

