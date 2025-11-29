#include "core/math/math.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace math {


    float sin(float x) {
        return glm::sin(x);
    }

    float cos(float x) {
        return glm::cos(x);
    }

    float tan(float x) {
        return glm::tan(x);
    }

    float asin(float x) {
        return glm::asin(x);
    }

    float acos(float x) {
        return glm::acos(x);
    }

    float atan(float x) {
        return glm::atan(x);
    }

    float atan2(float y, float x) {
        return glm::atan(y, x);
    }

    float sinh(float x) {
        return glm::sinh(x);
    }

    float cosh(float x) {
        return glm::cosh(x);
    }

    float tanh(float x) {
        return glm::tanh(x);
    }

    float exp(float x) {
        return glm::exp(x);
    }

    float log(float x) {
        return glm::log(x);
    }

    float log2(float x) {
        return glm::log2(x);
    }

    float log10(float x) {
        return std::log10(x);
    }

    float pow(float base, float exp) {
        return glm::pow(base, exp);
    }

    float sqrt(float x) {
        return glm::sqrt(x);
    }

    float cbrt(float x) {
        return std::cbrt(x);
    }


    float floor(float x) {
        return glm::floor(x);
    }

    float ceil(float x) {
        return glm::ceil(x);
    }

    float round(float x) {
        return glm::round(x);
    }

    float trunc(float x) {
        return glm::trunc(x);
    }



    float abs(float x) {
        return glm::abs(x);
    }

    int abs(int x) {
        return glm::abs(x);
    }


    float deg2rad(float degrees) {
        return glm::radians(degrees);
    }

    float rad2deg(float radians) {
        return glm::degrees(radians);
    }



    bool is_zero_approx(float x) {
        return abs(x) < EPSILON;
    }

    bool is_equal_approx(float a, float b) {
        return abs(a - b) < EPSILON;
    }



    float random(float min_val, float max_val) {
        return min_val + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max_val - min_val)));
    }

    int random(int min_val, int max_val) {
        return min_val + (rand() % (max_val - min_val + 1));
    }


    float snap(float value, float step) {
        if (step != 0.0f) {
            return floor(value / step + 0.5f) * step;
        }
        return value;
    }

} // namespace math

