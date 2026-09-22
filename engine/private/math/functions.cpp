#include "math/functions.h"


namespace golias {

    float Math_Sin(float value) {
        return std::sin(value);
    }

    float Math_Cos(float value) {
        return std::cos(value);
    }

    float Math_Tan(float value) {
        return std::tan(value);
    }

    float Math_Asin(float value) {
        return std::asin(value);
    }

    float Math_Acos(float value) {
        return std::acos(value);
    }

    float Math_Atan(float value) {
        return std::atan(value);
    }

    float Math_Atan2(float y, float x) {
        return std::atan2(y, x);
    }

    float Math_Sqrt(float value) {
        return std::sqrt(value);
    }

    float Math_Abs(float value) {
        return std::fabs(value);
    }

    float Math_Floor(float value) {
        return std::floor(value);
    }

    float Math_Ceil(float value) {
        return std::ceil(value);
    }

    float Math_Round(float value) {
        return std::round(value);
    }

    float Math_Min(float a, float b) {
        return std::fmin(a, b);
    }

    float Math_Max(float a, float b) {
        return std::fmax(a, b);
    }

    float Math_Clamp(float value, float min, float max) {
        return std::fmax(min, std::fmin(value, max));
    }

    float Math_Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    float Math_InverseLerp(float a, float b, float value) {
        if (Math_NearlyEqual(a, b)) {
            return 0.0f;
        }

        return (value - a) / (b - a);
    }

    float Math_SmoothStep(float a, float b, float t) {
        t = Math_Clamp(t, 0.0f, 1.0f);
        t = t * t * (3.0f - 2.0f * t);

        return Math_Lerp(a, b, t);
    }

    float Math_MoveTowards(float current, float target, float maxDelta) {
        const float delta = target - current;

        if (Math_Abs(delta) <= maxDelta) {
            return target;
        }

        return current + Math_Sign(delta) * maxDelta;
    }

    float Math_Repeat(float value, float length) {
        if (length <= 0.0f) {
            return 0.0f;
        }

        return value - Math_Floor(value / length) * length;
    }

    float Math_Pow(float base, float exponent) {
        return std::pow(base, exponent);
    }

    float Math_Log(float value) {
        return std::log(value);
    }

    float Math_Log10(float value) {
        return std::log10(value);
    }

    float Math_Exp(float value) {
        return std::exp(value);
    }

    float Math_Sign(float value) {
        if (value > 0.0f) {
            return 1.0f;
        }

        if (value < 0.0f) {
            return -1.0f;
        }

        return 0.0f;
    }

    float Math_Radians(float degrees) {
        return degrees * (Math_Pi / 180.0f);
    }

    float Math_Degrees(float radians) {
        return radians * (180.0f / Math_Pi);
    }

    bool Math_IsZero(float value, float epsilon) {
        return Math_Abs(value) <= epsilon;
    }

    bool Math_NearlyEqual(float a, float b, float epsilon) {
        return Math_Abs(a - b) <= epsilon;
    }

} // namespace golias
