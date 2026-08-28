#pragma once
#include "stdafx.h"

namespace golias {

    struct Viewport {
        int X = 0, Y = 0, Width = 0, Height = 0;
    };

    struct Color {
        float R = 0.0f, G = 0.0f, B = 0.0f, A = 1.0f;

        static Color White() {
            return {1.0f, 1.0f, 1.0f, 1.0f};
        }
    };

    enum class CullMode { None, Front, Back, FrontAndBack };
    enum class ClearFlag : uint32_t { Color = 1 << 0, Depth = 1 << 1, Stencil = 1 << 2 };

    inline ClearFlag operator|(ClearFlag a, ClearFlag b) {
        return static_cast<ClearFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline ClearFlag operator&(ClearFlag a, ClearFlag b) {
        return static_cast<ClearFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    inline ClearFlag& operator|=(ClearFlag& a, ClearFlag b) {
        return a = a | b;
    }

    inline bool HasFlag(ClearFlag value, ClearFlag flag) {
        return static_cast<uint32_t>(value & flag) != 0;
    }

} // namespace golias
