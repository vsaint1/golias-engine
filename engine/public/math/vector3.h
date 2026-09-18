#pragma once

namespace golias {

    class Vector3 {
    public:
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vector3() = default;

        Vector3(float x, float y, float z) : x(x), y(y), z(z) {
        }
    };
} // namespace golias
