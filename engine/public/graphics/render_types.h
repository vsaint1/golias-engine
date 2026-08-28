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


    // clang-format off

    enum class BlendFactor : uint8_t {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstColor,
        OneMinusDstColor,
        DstAlpha,
        OneMinusDstAlpha
    };

    enum class FrontFace { 
        CounterClockWise, 
        ClockWise 
    };

    enum class CullMode { 
        None, 
        Front,
        Back, 
        FrontAndBack 
    };

    enum class ClearFlag : uint32_t { 
        Color = 1 << 0, 
        Depth = 1 << 1, 
        Stencil = 1 << 2 
    };

    enum class BufferTarget : uint32_t { 
        Vertex = 1 << 0, 
        Index = 1 << 1, 
        Uniform = 1 << 2, 
        Storage = 1 << 3, 
        Indirect = 1 << 4 
    };

    enum class BufferUsage : uint32_t {
        Static,
        Dynamic,
        Stream
    };

    struct BufferDesc{
        BufferTarget Target = BufferTarget::Vertex;
        BufferUsage Usage = BufferUsage::Static;
        size_t Size = 0;
    };

    // clang-format on


    inline constexpr BufferTarget operator|(BufferTarget a, BufferTarget b) {
        return static_cast<BufferTarget>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline constexpr bool HasFlag(BufferTarget value, BufferTarget flag) {
        return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
    }


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
