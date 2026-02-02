#pragma once
#include <SDL3/SDL_stdinc.h>
#include <string>
#include <vector>

#include <glm/glm.hpp>


enum class ETextureWrapMode { REPEAT, CLAMP_TO_EDGE, MIRRORED_REPEAT, CLAMP_TO_BORDER };

enum class ETextureFilterMode {
    NEAREST,
    BILINEAR,
    TRILINEAR,
};

enum class ETextureMipGenSettings {
    NO_MIPMAPS, // No mipmaps
    FROM_TEXTURE_GROUP, // Use texture group settings (not implemented here)
    SIMPLE_AVERAGE, // Generate mipmaps using simple averaging
    SHARPEN0, // Future: Sharpen filter level 0
    SHARPEN1, // Future: Sharpen filter level 1
};

enum class EFramebufferAttachment {
    COLOR_ATTACHMENT0 = 0,
    COLOR_ATTACHMENT1 = 1,
    COLOR_ATTACHMENT2 = 2,
    COLOR_ATTACHMENT3 = 3,
    DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT
};

enum class ETextureFormat { R8, RG8, RGBA8, RGB8, RGBA16F, RGB16F, RGBA32F, RGB32F, DEPTH24, DEPTH32F, DEPTH24_STENCIL8, DEPTH32F_STENCIL8 };

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
    CULL_MODE_DISABLED,
    CULL_MODE_FRONT,
    CULL_MODE_BACK,
    CULL_MODE_FRONT_AND_BACK,
};

enum class EPolygonMode { FILL, LINE, POINT };

enum class EPrimitiveTopology { POINTS, LINES, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN };

enum class EShaderStage { VERTEX = 1 << 0, FRAGMENT = 1 << 1, COMPUTE = 1 << 2 };

enum class EBlendMode { BLEND_MODE_DISABLED, BLEND_MODE_OPAQUE, BLEND_MODE_ALPHA, BLEND_MODE_ADDITIVE, BLEND_MODE_MULTIPLY };

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

enum EClearFlags : uint32_t { CLEAR_COLOR = 1 << 0, CLEAR_DEPTH = 1 << 1, CLEAR_STENCIL = 1 << 2 };

inline EClearFlags operator|(EClearFlags a, EClearFlags b) {
    return static_cast<EClearFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline EClearFlags& operator|=(EClearFlags& a, EClearFlags b) {
    a = a | b;
    return a;
}

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
    class Shader;

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



    struct RasterizerState {
        ECullMode cullMode         = ECullMode::CULL_MODE_BACK;
        EPolygonMode polygonMode   = EPolygonMode::FILL;
        bool frontFaceCCW          = true;
        bool depthBiasEnable       = false;
        float depthBiasConstant    = 0.0f;
        float depthBiasSlopeFactor = 0.0f;

        bool operator==(const RasterizerState& other) const = default;
    };

    struct DepthStencilState {
        bool depthTestEnable       = true;
        bool depthWriteEnable      = true;
        EComparisonFunc depthFunc  = EComparisonFunc::COMPARISON_LESS;
        bool stencilTestEnable     = false;
        EStencilOp stencilFailOp   = EStencilOp::STENCIL_OP_KEEP;
        EStencilOp depthFailOp     = EStencilOp::STENCIL_OP_KEEP;
        EStencilOp passOp          = EStencilOp::STENCIL_OP_KEEP;
        EComparisonFunc stencilFunc = EComparisonFunc::COMPARISON_ALWAYS;
        uint32_t stencilReadMask   = 0xFF;
        uint32_t stencilWriteMask  = 0xFF;
        uint32_t stencilRef        = 0;

        bool operator==(const DepthStencilState& other) const = default;
    };

    struct BlendAttachmentState {
        bool blendEnable            = false;
        EBlendFactor srcColorBlend  = EBlendFactor::BLEND_ONE;
        EBlendFactor dstColorBlend  = EBlendFactor::BLEND_ZERO;
        EBlendOp colorBlendOp       = EBlendOp::BLEND_OP_ADD;
        EBlendFactor srcAlphaBlend  = EBlendFactor::BLEND_ONE;
        EBlendFactor dstAlphaBlend  = EBlendFactor::BLEND_ZERO;
        EBlendOp alphaBlendOp       = EBlendOp::BLEND_OP_ADD;
        bool writeR                 = true;
        bool writeG                 = true;
        bool writeB                 = true;
        bool writeA                 = true;

        bool operator==(const BlendAttachmentState& other) const = default;
    };

    struct BlendState {
        std::vector<BlendAttachmentState> attachments;
        glm::vec4 blendConstants = glm::vec4(0.0f);

        BlendState() {
            attachments.resize(1);
        }

        bool operator==(const BlendState& other) const = default;
    };

    struct PipelineState {
        RasterizerState rasterizer;
        DepthStencilState depthStencil;
        BlendState blend;
        EPrimitiveTopology topology = EPrimitiveTopology::TRIANGLES;

        bool operator==(const PipelineState& other) const = default;
    };

    enum class ELoadOp {
        LOAD,
        CLEAR,
        DONT_CARE
    };

    enum class EStoreOp {
        STORE,
        DONT_CARE
    };

    struct ClearValue {
        union {
            struct {
                float r, g, b, a;
            } color;
            struct {
                float depth;
                uint32_t stencil;
            } depthStencil;
        };

        ClearValue() : color{0.0f, 0.0f, 0.0f, 1.0f} {}
        
        static ClearValue Color(float r, float g, float b, float a = 1.0f) {
            ClearValue val;
            val.color = {r, g, b, a};
            return val;
        }

        static ClearValue Color(const glm::vec4& col) {
            return Color(col.r, col.g, col.b, col.a);
        }

        static ClearValue DepthStencil(float depth, uint32_t stencil = 0) {
            ClearValue val;
            val.depthStencil = {depth, stencil};
            return val;
        }
    };

    struct RenderPassBeginInfo {
        Framebuffer* framebuffer = nullptr;
        std::vector<ClearValue> clearValues;
        Viewport viewport;
        Scissor scissor;
        ELoadOp colorLoadOp = ELoadOp::CLEAR;
        EStoreOp colorStoreOp = EStoreOp::STORE;
        ELoadOp depthLoadOp = ELoadOp::CLEAR;
        EStoreOp depthStoreOp = EStoreOp::STORE;
    };

    ETextureFormat TextureFormatFromChannels(int channels);
    int ChannelsFromTextureFormat(ETextureFormat format);
}; // namespace golias
