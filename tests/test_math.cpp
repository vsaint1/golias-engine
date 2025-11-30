#include <gtest/gtest.h>
#include "core/math/math.h"

TEST(Vector2Test, Construction) {
    Vector2 v1;
    EXPECT_FLOAT_EQ(v1.x, 0.0f);
    EXPECT_FLOAT_EQ(v1.y, 0.0f);

    Vector2 v2(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v2.x, 3.0f);
    EXPECT_FLOAT_EQ(v2.y, 4.0f);
}

TEST(Vector2Test, Addition) {
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 result = a + b;

    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
}

TEST(Vector2Test, Subtraction) {
    Vector2 a(5.0f, 7.0f);
    Vector2 b(2.0f, 3.0f);
    Vector2 result = a - b;

    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
}

TEST(Vector2Test, Multiplication) {
    Vector2 v(2.0f, 3.0f);
    Vector2 result = v * 2.0f;

    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
}

TEST(Vector2Test, Division) {
    Vector2 v(10.0f, 20.0f);
    Vector2 result = v / 2.0f;

    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
}

TEST(Vector2Test, Length) {
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);
}

TEST(Vector2Test, Normalize) {
    Vector2 v(3.0f, 4.0f);
    Vector2 normalized = v.normalized();

    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
}

TEST(Vector2Test, DotProduct) {
    Vector2 a(2.0f, 3.0f);
    Vector2 b(4.0f, 5.0f);

    EXPECT_FLOAT_EQ(a.dot(b), 23.0f);
}

TEST(Vector2Test, Distance) {
    Vector2 a(1.0f, 2.0f);
    Vector2 b(4.0f, 6.0f);

    EXPECT_FLOAT_EQ(a.distance_to(b), 5.0f);
}

TEST(Vector2Test, Lerp) {
    Vector2 a(0.0f, 0.0f);
    Vector2 b(10.0f, 10.0f);
    Vector2 mid = a.lerp(b, 0.5f);

    EXPECT_FLOAT_EQ(mid.x, 5.0f);
    EXPECT_FLOAT_EQ(mid.y, 5.0f);
}

TEST(MathTest, Clamp) {
    EXPECT_EQ(math::clamp(5,0,10),5);
    EXPECT_EQ(math::clamp(5, 0, 10), 5);
    EXPECT_EQ(math::clamp(-5, 0, 10), 0);
    EXPECT_EQ(math::clamp(15, 0, 10), 10);
}

TEST(MathTest, Lerp) {
    EXPECT_FLOAT_EQ(math::lerp(0.0f, 10.0f, 0.5f), 5.0f);
    EXPECT_FLOAT_EQ(math::lerp(0.0f, 10.0f, 0.0f), 0.0f);
    EXPECT_FLOAT_EQ(math::lerp(0.0f, 10.0f, 1.0f), 10.0f);
}

TEST(MathTest, Abs) {
    EXPECT_EQ(math::abs(-5), 5);
    EXPECT_EQ(math::abs(5), 5);
    EXPECT_FLOAT_EQ(math::abs(-3.14f), 3.14f);
}

TEST(MathTest, MinMax) {
    EXPECT_EQ(math::min(5, 10), 5);
    EXPECT_EQ(math::max(5, 10), 10);
    EXPECT_FLOAT_EQ(math::min(3.14f, 2.71f), 2.71f);
}

TEST(MathTest, Sign) {
    EXPECT_EQ(math::sign(10), 1);
    EXPECT_EQ(math::sign(-10), -1);
    EXPECT_EQ(math::sign(0), 0);
}

TEST(Vector3Test, Construction) {
    Vector3 v1;
    EXPECT_FLOAT_EQ(v1.x, 0.0f);
    EXPECT_FLOAT_EQ(v1.y, 0.0f);
    EXPECT_FLOAT_EQ(v1.z, 0.0f);

    Vector3 v2(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v2.x, 1.0f);
    EXPECT_FLOAT_EQ(v2.y, 2.0f);
    EXPECT_FLOAT_EQ(v2.z, 3.0f);
}

TEST(Vector3Test, Addition) {
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 result = a + b;

    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST(Vector3Test, Subtraction) {
    Vector3 a(10.0f, 8.0f, 6.0f);
    Vector3 b(3.0f, 2.0f, 1.0f);
    Vector3 result = a - b;

    EXPECT_FLOAT_EQ(result.x, 7.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 5.0f);
}

TEST(Vector3Test, Multiplication) {
    Vector3 v(2.0f, 3.0f, 4.0f);
    Vector3 result = v * 2.0f;

    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 8.0f);
}

TEST(Vector3Test, Division) {
    Vector3 v(10.0f, 20.0f, 30.0f);
    Vector3 result = v / 2.0f;

    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
    EXPECT_FLOAT_EQ(result.z, 15.0f);
}

TEST(Vector3Test, Length) {
    Vector3 v(2.0f, 3.0f, 6.0f);
    EXPECT_FLOAT_EQ(v.length(), 7.0f);
}

TEST(Vector3Test, Normalize) {
    Vector3 v(3.0f, 4.0f, 0.0f);
    Vector3 normalized = v.normalized();

    EXPECT_NEAR(normalized.length(), 1.0f, 0.0001f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
    EXPECT_FLOAT_EQ(normalized.z, 0.0f);
}

TEST(Vector3Test, DotProduct) {
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);

    EXPECT_FLOAT_EQ(a.dot(b), 32.0f);
}

TEST(Vector3Test, CrossProduct) {
    Vector3 a(1.0f, 0.0f, 0.0f);
    Vector3 b(0.0f, 1.0f, 0.0f);
    Vector3 result = a.cross(b);

    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 1.0f);
}

TEST(Vector3Test, Distance) {
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 6.0f, 3.0f);

    EXPECT_FLOAT_EQ(a.distance_to(b), 5.0f);
}

TEST(Vector3Test, Lerp) {
    Vector3 a(0.0f, 0.0f, 0.0f);
    Vector3 b(10.0f, 20.0f, 30.0f);
    Vector3 mid = a.lerp(b, 0.5f);

    EXPECT_FLOAT_EQ(mid.x, 5.0f);
    EXPECT_FLOAT_EQ(mid.y, 10.0f);
    EXPECT_FLOAT_EQ(mid.z, 15.0f);
}


TEST(Vector4Test, Construction) {
    Vector4 v1;
    EXPECT_FLOAT_EQ(v1.x, 0.0f);
    EXPECT_FLOAT_EQ(v1.y, 0.0f);
    EXPECT_FLOAT_EQ(v1.z, 0.0f);
    EXPECT_FLOAT_EQ(v1.w, 0.0f);

    Vector4 v2(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v2.x, 1.0f);
    EXPECT_FLOAT_EQ(v2.y, 2.0f);
    EXPECT_FLOAT_EQ(v2.z, 3.0f);
    EXPECT_FLOAT_EQ(v2.w, 4.0f);
}

TEST(Vector4Test, Addition) {
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 result = a + b;

    EXPECT_FLOAT_EQ(result.x, 6.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 10.0f);
    EXPECT_FLOAT_EQ(result.w, 12.0f);
}

TEST(Vector4Test, Subtraction) {
    Vector4 a(10.0f, 8.0f, 6.0f, 4.0f);
    Vector4 b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 result = a - b;

    EXPECT_FLOAT_EQ(result.x, 9.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
    EXPECT_FLOAT_EQ(result.w, 0.0f);
}

TEST(Vector4Test, Multiplication) {
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 result = v * 2.0f;

    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
    EXPECT_FLOAT_EQ(result.w, 8.0f);
}

TEST(Vector4Test, Division) {
    Vector4 v(10.0f, 20.0f, 30.0f, 40.0f);
    Vector4 result = v / 2.0f;

    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
    EXPECT_FLOAT_EQ(result.z, 15.0f);
    EXPECT_FLOAT_EQ(result.w, 20.0f);
}

TEST(Vector4Test, DotProduct) {
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);

    EXPECT_FLOAT_EQ(a.dot(b), 70.0f); // 1*5 + 2*6 + 3*7 + 4*8
}

TEST(Vector4Test, Length) {
    Vector4 v(2.0f, 2.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.length(), 3.0f);
}


TEST(QuaternionTest, Construction) {
    Quaternion q1;
    EXPECT_FLOAT_EQ(q1.x, 0.0f);
    EXPECT_FLOAT_EQ(q1.y, 0.0f);
    EXPECT_FLOAT_EQ(q1.z, 0.0f);
    EXPECT_FLOAT_EQ(q1.w, 1.0f);

    Quaternion q2(4.0f, 1.0f, 2.0f, 3.0f); // w, x, y, z
    EXPECT_FLOAT_EQ(q2.w, 4.0f);
    EXPECT_FLOAT_EQ(q2.x, 1.0f);
    EXPECT_FLOAT_EQ(q2.y, 2.0f);
    EXPECT_FLOAT_EQ(q2.z, 3.0f);
}

TEST(QuaternionTest, Identity) {
    Quaternion identity = Quaternion::IDENTITY;
    EXPECT_FLOAT_EQ(identity.x, 0.0f);
    EXPECT_FLOAT_EQ(identity.y, 0.0f);
    EXPECT_FLOAT_EQ(identity.z, 0.0f);
    EXPECT_FLOAT_EQ(identity.w, 1.0f);
}

TEST(QuaternionTest, Multiplication) {
    Quaternion q1(0.0f, 0.0f, 0.707f, 0.707f);
    Quaternion q2(0.0f, 0.0f, 0.707f, 0.707f);
    Quaternion result = q1 * q2;

    EXPECT_NEAR(result.x, 0.0f, 0.01f);
    EXPECT_NEAR(result.y, 0.0f, 0.01f);
    EXPECT_NEAR(result.z, 1.0f, 0.01f);
    EXPECT_NEAR(result.w, 0.0f, 0.01f);
}

TEST(QuaternionTest, Normalize) {
    Quaternion q(4.0f, 1.0f, 2.0f, 3.0f); // w, x, y, z
    Quaternion normalized = q.normalized();

    float length = math::sqrt(normalized.x * normalized.x +
                             normalized.y * normalized.y +
                             normalized.z * normalized.z +
                             normalized.w * normalized.w);
    EXPECT_NEAR(length, 1.0f, 0.0001f);
}

TEST(QuaternionTest, Conjugate) {
    Quaternion q(4.0f, 1.0f, 2.0f, 3.0f); // w, x, y, z
    Quaternion conj = q.conjugate();

    EXPECT_FLOAT_EQ(conj.w, 4.0f);
    EXPECT_FLOAT_EQ(conj.x, -1.0f);
    EXPECT_FLOAT_EQ(conj.y, -2.0f);
    EXPECT_FLOAT_EQ(conj.z, -3.0f);
}

TEST(QuaternionTest, Inverse) {
    Quaternion q(0.0f, 0.707f, 0.0f, 0.707f);
    Quaternion inv = q.inverse();
    Quaternion result = q * inv;

    // q * q^-1 should equal identity
    EXPECT_NEAR(result.x, 0.0f, 0.0001f);
    EXPECT_NEAR(result.y, 0.0f, 0.0001f);
    EXPECT_NEAR(result.z, 0.0f, 0.0001f);
    EXPECT_NEAR(result.w, 1.0f, 0.0001f);
}

TEST(QuaternionTest, FromAxisAngle) {
    Vector3 axis(0.0f, 1.0f, 0.0f); // Y axis
    float angle = math::deg2rad(90.0f);
    Quaternion q = Quaternion::from_axis_angle(axis, angle);

    EXPECT_NEAR(q.x, 0.0f, 0.0001f);
    EXPECT_NEAR(q.y, 0.707f, 0.01f);
    EXPECT_NEAR(q.z, 0.0f, 0.0001f);
    EXPECT_NEAR(q.w, 0.707f, 0.01f);
}

TEST(QuaternionTest, ToEuler) {
    Quaternion q = Quaternion::from_euler(Vector3(0.0f, math::deg2rad(90.0f), 0.0f));
    Vector3 euler = q.to_euler();

    EXPECT_NEAR(euler.x, 0.0f, 0.01f);
    EXPECT_NEAR(euler.y, math::deg2rad(90.0f), 0.01f);
    EXPECT_NEAR(euler.z, 0.0f, 0.01f);
}

TEST(QuaternionTest, Slerp) {
    Quaternion a = Quaternion::IDENTITY;
    Quaternion b = Quaternion::from_euler(Vector3(0.0f, math::deg2rad(90.0f), 0.0f));
    Quaternion mid = a.slerp(b, 0.5f);

    Vector3 euler = mid.to_euler();
    EXPECT_NEAR(euler.y, math::deg2rad(45.0f), 0.01f);
}

TEST(QuaternionTest, DotProduct) {
    Quaternion a(0.0f, 1.0f, 0.0f, 0.0f); // w, x, y, z
    Quaternion b(0.0f, 1.0f, 0.0f, 0.0f);

    EXPECT_FLOAT_EQ(a.dot(b), 1.0f);
}


TEST(Matrix4Test, Construction) {
    Matrix4 m;
    // Default should be identity
    EXPECT_FLOAT_EQ(m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m[2][2], 1.0f);
    EXPECT_FLOAT_EQ(m[3][3], 1.0f);
}

TEST(Matrix4Test, Identity) {
    Matrix4 identity = Matrix4::IDENTITY;

    EXPECT_FLOAT_EQ(identity[0][0], 1.0f);
    EXPECT_FLOAT_EQ(identity[1][1], 1.0f);
    EXPECT_FLOAT_EQ(identity[2][2], 1.0f);
    EXPECT_FLOAT_EQ(identity[3][3], 1.0f);

    EXPECT_FLOAT_EQ(identity[0][1], 0.0f);
    EXPECT_FLOAT_EQ(identity[1][0], 0.0f);
}

TEST(Matrix4Test, Multiplication) {
    Matrix4 a = Matrix4::IDENTITY;
    Matrix4 b = Matrix4::IDENTITY;
    Matrix4 result = a * b;

    EXPECT_FLOAT_EQ(result[0][0], 1.0f);
    EXPECT_FLOAT_EQ(result[1][1], 1.0f);
    EXPECT_FLOAT_EQ(result[2][2], 1.0f);
    EXPECT_FLOAT_EQ(result[3][3], 1.0f);
}

TEST(Matrix4Test, Translation) {
    Matrix4 trans = Matrix4::translate(Vector3(10.0f, 20.0f, 30.0f));

    EXPECT_FLOAT_EQ(trans[3][0], 10.0f);
    EXPECT_FLOAT_EQ(trans[3][1], 20.0f);
    EXPECT_FLOAT_EQ(trans[3][2], 30.0f);
}

TEST(Matrix4Test, Scale) {
    Matrix4 scale = Matrix4::scale(Vector3(2.0f, 3.0f, 4.0f));

    EXPECT_FLOAT_EQ(scale[0][0], 2.0f);
    EXPECT_FLOAT_EQ(scale[1][1], 3.0f);
    EXPECT_FLOAT_EQ(scale[2][2], 4.0f);
}

TEST(Matrix4Test, RotationX) {
    Matrix4 rot = Matrix4::rotate(math::deg2rad(90.0f), Vector3(1.0f, 0.0f, 0.0f));
    Vector3 v(0.0f, 1.0f, 0.0f);
    Vector3 result = rot.xform(v);

    EXPECT_NEAR(result.x, 0.0f, 0.0001f);
    EXPECT_NEAR(result.y, 0.0f, 0.0001f);
    EXPECT_NEAR(result.z, 1.0f, 0.0001f);
}

TEST(Matrix4Test, RotationY) {
    Matrix4 rot = Matrix4::rotate(math::deg2rad(90.0f), Vector3(0.0f, 1.0f, 0.0f));
    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 result = rot.xform(v);

    EXPECT_NEAR(result.x, 0.0f, 0.0001f);
    EXPECT_NEAR(result.y, 0.0f, 0.0001f);
    EXPECT_NEAR(result.z, -1.0f, 0.0001f);
}

TEST(Matrix4Test, RotationZ) {
    Matrix4 rot = Matrix4::rotate(math::deg2rad(90.0f), Vector3(0.0f, 0.0f, 1.0f));
    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 result = rot.xform(v);

    EXPECT_NEAR(result.x, 0.0f, 0.0001f);
    EXPECT_NEAR(result.y, 1.0f, 0.0001f);
    EXPECT_NEAR(result.z, 0.0f, 0.0001f);
}

TEST(Matrix4Test, Transpose) {
    Matrix4 m = Matrix4::IDENTITY;
    m[0][1] = 5.0f;
    m[1][0] = 10.0f;

    Matrix4 transposed = m.transposed();

    EXPECT_FLOAT_EQ(transposed[0][1], 10.0f);
    EXPECT_FLOAT_EQ(transposed[1][0], 5.0f);
}

TEST(Matrix4Test, Determinant) {
    Matrix4 m = Matrix4::IDENTITY;
    EXPECT_FLOAT_EQ(m.determinant(), 1.0f);

    Matrix4 scaled = Matrix4::scale(Vector3(2.0f, 2.0f, 2.0f));
    EXPECT_FLOAT_EQ(scaled.determinant(), 8.0f); // 2 * 2 * 2
}

TEST(Matrix4Test, Inverse) {
    Matrix4 trans = Matrix4::translate(Vector3(10.0f, 20.0f, 30.0f));
    Matrix4 inv = trans.inverse();
    Matrix4 result = trans * inv;


    EXPECT_NEAR(result[0][0], 1.0f, 0.0001f);
    EXPECT_NEAR(result[1][1], 1.0f, 0.0001f);
    EXPECT_NEAR(result[2][2], 1.0f, 0.0001f);
    EXPECT_NEAR(result[3][3], 1.0f, 0.0001f);
}

TEST(Matrix4Test, LookAt) {
    Vector3 eye(0.0f, 0.0f, 5.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix4 view = Matrix4::look_at(eye, target, up);

    EXPECT_NE(view[0][0], 0.0f);
}

TEST(Matrix4Test, Perspective) {
    float fov = math::deg2rad(60.0f);
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 100.0f;

    Matrix4 proj = Matrix4::perspective(fov, aspect, near, far);

    EXPECT_NE(proj[0][0], 0.0f);
    EXPECT_NE(proj[1][1], 0.0f);
}

TEST(Matrix4Test, Orthographic) {
    Matrix4 ortho = Matrix4::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    EXPECT_NE(ortho[0][0], 0.0f);
    EXPECT_NE(ortho[1][1], 0.0f);
}

