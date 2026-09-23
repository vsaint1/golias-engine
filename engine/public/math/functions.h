#pragma once

#include <cmath>


namespace golias {

    constexpr float Math_Pi      = 3.14159265358979323846f;
    constexpr float Math_TwoPi   = 6.28318530717958647692f;
    constexpr float Math_HalfPi  = 1.57079632679489661923f;
    constexpr float Math_Epsilon = 1e-6f;

    float Math_Sin(float value);
    float Math_Cos(float value);
    float Math_Tan(float value);

    float Math_Asin(float value);
    float Math_Acos(float value);
    float Math_Atan(float value);
    float Math_Atan2(float y, float x);

    float Math_Sqrt(float value);

    float Math_Abs(float value);

    float Math_Floor(float value);
    float Math_Ceil(float value);
    float Math_Round(float value);

    float Math_Min(float a, float b);
    float Math_Max(float a, float b);

    float Math_Clamp(float value, float min, float max);

    float Math_Lerp(float a, float b, float t);
    float Math_InverseLerp(float a, float b, float value);
    float Math_SmoothStep(float a, float b, float t);

    float Math_MoveTowards(float current, float target, float max_delta);

    float Math_Repeat(float value, float length);

    float Math_Pow(float base, float exponent);
    float Math_Log(float value);
    float Math_Log10(float value);
    float Math_Exp(float value);

    float Math_Sign(float value);

    float Math_Radians(float degrees);
    float Math_Degrees(float radians);

    bool Math_IsZero(float value, float epsilon = Math_Epsilon);
    bool Math_NearlyEqual(float a, float b, float epsilon = Math_Epsilon);

} // namespace golias
