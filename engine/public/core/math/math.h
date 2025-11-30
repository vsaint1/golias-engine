#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/math/quaternion.h"
#include "core/math/basis.h"
#include "core/math/matrix4.h"

#include <glm/gtc/type_ptr.hpp>

namespace math {
    constexpr float PI       = 3.14159265358979323846f;
    constexpr float TAU      = 6.28318530717958647692f;
    constexpr float E        = 2.71828182845904523536f;
    constexpr float DEG2RAD  = PI / 180.0f;
    constexpr float RAD2DEG  = 180.0f / PI;
    constexpr float EPSILON  = 1e-6f;
    constexpr float INF      = __builtin_huge_valf();

    float sin(float x);
    float cos(float x);
    float tan(float x);
    float asin(float x);
    float acos(float x);
    float atan(float x);
    float atan2(float y, float x);

    float sinh(float x);
    float cosh(float x);
    float tanh(float x);

    float exp(float x);
    float log(float x);
    float log2(float x);
    float log10(float x);
    float pow(float base, float exp);
    float sqrt(float x);
    float cbrt(float x);

    float floor(float x);
    float ceil(float x);
    float round(float x);
    float trunc(float x);

    template<typename T>
    inline T min(T a, T b) { return (a < b) ? a : b; }

    template<typename T>
    inline T max(T a, T b) { return (a > b) ? a : b; }

    template<typename T>
    inline T clamp(T value, T min_val, T max_val) {
        return min(max(value, min_val), max_val);
    }

    float abs(float x);
    int abs(int x);

    template<typename T>
    inline T sign(T x) { return (x > T(0)) ? T(1) : ((x < T(0)) ? T(-1) : T(0)); }

    template<typename T>
    inline T lerp(T a, T b, float t) {
        return a + (b - a) * t;
    }

    template<typename T>
    inline T smoothstep(T edge0, T edge1, T x) {
        T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
        return t * t * (T(3) - T(2) * t);
    }

    float deg2rad(float degrees);
    float rad2deg(float radians);

    bool is_zero_approx(float x);
    bool is_equal_approx(float a, float b);

    float random(float min_val, float max_val);
    int random(int min_val, int max_val);

    template<typename T>
    inline T move_toward(T current, T target, T delta) {
        if (abs(target - current) <= delta) {
            return target;
        }
        return current + sign(target - current) * delta;
    }

    float snap(float value, float step);

}



