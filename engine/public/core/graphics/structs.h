#pragma once
#include <SDL3/SDL_stdinc.h>
#include <string>
#include <vector>

#include <glm/glm.hpp>


enum class ETextureFormat { RGB, RGBA, RED, RG };

enum class ETextureWrapMode { REPEAT, CLAMP_TO_EDGE, MIRRORED_REPEAT, CLAMP_TO_BORDER };

enum class ETextureFilterMode {
    NEAREST,
    BILINEAR,
    TRILINEAR,
};

// Texture Mip Gen Settings
enum class ETextureMipGenSettings {
    NO_MIPMAPS, // No mipmaps
    FROM_TEXTURE_GROUP, // Use texture group settings (not implemented here)
    SIMPLE_AVERAGE, // Generate mipmaps using simple averaging
    SHARPEN0, // Future: Sharpen filter level 0
    SHARPEN1, // Future: Sharpen filter level 1
};


enum class ETextureCompressionSettings {
    DEFAULT,
    NORMALMAP,
    HIGH_QUALITY,
    LOW_QUALITY,
    HDR,
    MAX,
};

enum class ETextureFlags : int {
    NONE          = 0,
    HAS_ALBEDO    = 1 << 0, // 0x01
    HAS_METALLIC  = 1 << 1, // 0x02
    HAS_ROUGHNESS = 1 << 2, // 0x04
    HAS_NORMAL    = 1 << 3, // 0x08
    HAS_AO        = 1 << 4, // 0x10
    HAS_EMISSIVE  = 1 << 5 // 0x20
};

inline ETextureFlags operator|(ETextureFlags a, ETextureFlags b) {
    return static_cast<ETextureFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline ETextureFlags& operator|=(ETextureFlags& a, ETextureFlags b) {
    a = a | b;
    return a;
}

inline int operator&(ETextureFlags a, ETextureFlags b) {
    return static_cast<int>(a) & static_cast<int>(b);
}

enum class EBufferTarget : uint32_t {
    ARRAY_BUFFER         = 0x1, // GL_ARRAY_BUFFER
    ELEMENT_ARRAY_BUFFER = 0x2 // GL_ELEMENT_ARRAY_BUFFER
};

enum class EBufferUsageFlags : uint32_t {
    STATIC_DRAW  = 0x1, // GL_STATIC_DRAW
    DYNAMIC_DRAW = 0x2, // GL_DYNAMIC_DRAW
    STREAM_DRAW  = 0x3 // GL_STREAM_DRAW
};

struct Buffer {
    uint32_t handle = 0;
    size_t size     = 0;
    EBufferUsageFlags usage_flags;
    EBufferTarget target = EBufferTarget::ARRAY_BUFFER;
};


enum class EDataType : uint32_t {
    BYTE,
    UNSIGNED_BYTE,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    FLOAT

};

enum class EBlendMode { DISABLED, ALPHA, ADDITIVE, MULTIPLY };

struct Rect {
    int x      = 0;
    int y      = 0;
    int width  = 0;
    int height = 0;
};


namespace golias {

    struct VertexElement {
        Uint32 location; // Shader layout(location = X)
        Uint32 components; // Number of components (1..4)
        EDataType type; // GL_FLOAT, GL_INT, etc.
        bool normalized; // GL_TRUE / GL_FALSE
        Uint32 offset; // Byte offset in vertex

        static constexpr Uint32 POSITION_INDEX = 0;
        static constexpr Uint32 COLOR_INDEX    = 1;
        static constexpr Uint32 TEXCOORD_INDEX = 2;
        static constexpr Uint32 NORMAL_INDEX   = 3;
    };

    struct VertexLayout {
        std::vector<VertexElement> elements;
        Uint32 stride = 0; // Total vertex size in bytes
    };

}; // namespace golias
