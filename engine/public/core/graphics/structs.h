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


enum class ETextureType { TEXTURE_TYPE_2D, TEXTURE_TYPE_2D_ARRAY, TEXTURE_TYPE_3D, TEXTURE_TYPE_CUBEMAP, TEXTURE_TYPE_CUBEMAP_ARRAY };

enum class ETextureFilter { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR };

enum class ETextureWrap { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };

enum class EStencilOp {
    STENCIL_OP_KEEP,
    STENCIL_OP_ZERO,
    STENCIL_OP_REPLACE,
    STENCIL_OP_INCR_SAT,
    STENCIL_OP_DECR_SAT,
    STENCIL_OP_INVERT,
    STENCIL_OP_INCR,
    STENCIL_OP_DECR
};

enum class EComparisonFunc {
    COMPARISON_NEVER,
    COMPARISON_LESS,
    COMPARISON_EQUAL,
    COMPARISON_LESS_EQUAL,
    COMPARISON_GREATER,
    COMPARISON_NOT_EQUAL,
    COMPARISON_GREATER_EQUAL,
    COMPARISON_ALWAYS
};

enum class EBlendOp { BLEND_OP_ADD, BLEND_OP_SUBTRACT, BLEND_OP_REV_SUBTRACT, BLEND_OP_MIN, BLEND_OP_MAX };


enum class EBlendFactor {
    BLEND_ZERO,
    BLEND_ONE,
    BLEND_SRC_COLOR,
    BLEND_INV_SRC_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_INV_SRC_ALPHA,
    BLEND_DST_ALPHA,
    BLEND_INV_DST_ALPHA,
    BLEND_DST_COLOR,
    BLEND_INV_DST_COLOR
};


enum class ECullMode {
    CULL_NONE,
    CULL_FRONT,
    CULL_BACK
};

enum class EPolygonMode { FILL, LINE, POINT };

enum class EPrimitiveTopology { POINTS, LINES, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN };

enum class EShaderStage { VERTEX = 1 << 0, FRAGMENT = 1 << 1, COMPUTE = 1 << 2 };

enum class EBlendMode { DISABLED, ALPHA, ADDITIVE, MULTIPLY };

enum class EBufferTarget {
    BUFFER_USAGE_VERTEX       = 1 << 0,
    BUFFER_USAGE_INDEX        = 1 << 1,
    BUFFER_USAGE_UNIFORM      = 1 << 2,
    BUFFER_USAGE_STORAGE      = 1 << 3,
    BUFFER_USAGE_TRANSFER_SRC = 1 << 4,
    BUFFER_USAGE_TRANSFER_DST = 1 << 5
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
    EBufferTarget target = EBufferTarget::BUFFER_USAGE_UNIFORM;
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


struct Viewport {
    int32_t x = 0, y = 0, width = 800, height = 600;
    float min_depth = 0.0f, max_depth = 1.0f;
};

struct Scissor {
    int32_t x = 0, y = 0;
    uint32_t width = 800, height = 600;
};


namespace golias {

    class Framebuffer;

    struct VertexElement {
        Uint32 location; // Shader layout(location = X)
        Uint32 components; // Number of components (1..4)
        EDataType type; // GL_FLOAT, GL_INT, etc.
        bool normalized; // GL_TRUE / GL_FALSE
        Uint32 offset; // Byte offset in vertex

        static constexpr Uint32 POSITION_INDEX     = 0;
        static constexpr Uint32 COLOR_INDEX        = 1;
        static constexpr Uint32 TEXCOORD_INDEX     = 2;
        static constexpr Uint32 NORMAL_INDEX       = 3;
        static constexpr Uint32 BONE_INDICES_INDEX = 4;
        static constexpr Uint32 BONE_WEIGHTS_INDEX = 5;
    };

    struct VertexLayout {
        std::vector<VertexElement> elements;
        Uint32 stride = 0; // Total vertex size in bytes
    };


}; // namespace golias
