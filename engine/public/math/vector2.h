#pragma once

namespace golias {

    class Vector2 {
    public:
        float x = 0.0f;
        float y = 0.0f;

        Vector2() = default;

        Vector2(float x, float y) : x(x), y(y) {
        }
    };
} // namespace golias
