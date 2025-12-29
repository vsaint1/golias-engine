#pragma once
#include <SDL3/SDL_stdinc.h>
#include <string>
#include <vector>

#include <glm/glm.hpp>


enum class ETextureFormat {
    RGB,
    RGBA,
    RED,
    RG
};

enum class ETextureWrapMode {
    REPEAT,
    CLAMP_TO_EDGE,
    MIRRORED_REPEAT,
    CLAMP_TO_BORDER
};

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

namespace golias {

    struct VertexElement {
        Uint32 location; // Shader layout(location = X)
        Uint32 components; // Number of components (1..4)
        EDataType type; // GL_FLOAT, GL_INT, etc.
        bool normalized; // GL_TRUE / GL_FALSE
        Uint32 offset; // Byte offset in vertex

        static constexpr int POSITION_INDEX = 0;
        static constexpr int COLOR_INDEX   = 1;
        static constexpr int TEXCOORD_INDEX = 2; 
        static constexpr int NORMAL_INDEX   = 3;
    };

    struct VertexLayout {
        std::vector<VertexElement> elements;
        Uint32 stride = 0; // Total vertex size in bytes
    };

}; // namespace golias
