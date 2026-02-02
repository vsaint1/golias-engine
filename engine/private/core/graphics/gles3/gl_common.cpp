#include "core/graphics/gles3/gl_common.h"

#include <spdlog/spdlog.h>


GLint ToGLBuferTarget(EBufferTarget target) {
    switch (target) {
    case EBufferTarget::BUFFER_USAGE_VERTEX:
        return GL_ARRAY_BUFFER;
    case EBufferTarget::BUFFER_USAGE_INDEX:
        return GL_ELEMENT_ARRAY_BUFFER;
    default:
        spdlog::warn("RenderingDeviceGLES3::ToGLBuferTarget: Unknown EBufferTarget enum value.");
        return GL_ARRAY_BUFFER;
    }
}

GLint ToGLBufferUsage(EBufferUsageFlags usage) {
    switch (usage) {
    case EBufferUsageFlags::STATIC_DRAW:
        return GL_STATIC_DRAW;
    case EBufferUsageFlags::DYNAMIC_DRAW:
        return GL_DYNAMIC_DRAW;
    case EBufferUsageFlags::STREAM_DRAW:
        return GL_STREAM_DRAW;
    default:
        spdlog::warn("RenderingDeviceGLES3::ToGLBufferUsage: Unknown EBufferUsageFlags enum value.");
        return GL_STATIC_DRAW;
    }
}

GLint ToGLDataType(EDataType type) {
    switch (type) {
    case EDataType::FLOAT:
        return GL_FLOAT;
    case EDataType::INT:
        return GL_INT;
    case EDataType::SHORT:
        return GL_SHORT;
    default:
        spdlog::warn("RenderingDeviceGLES3::ToGLDataType: Unknown EDataType enum value.");
        return GL_FLOAT;
    }
}


GLenum ToGLTextureWrapMode(ETextureWrapMode mode) {

    switch (mode) {
    case ETextureWrapMode::REPEAT:
        return GL_REPEAT;
    case ETextureWrapMode::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case ETextureWrapMode::MIRRORED_REPEAT:
        return GL_MIRRORED_REPEAT;
    case ETextureWrapMode::CLAMP_TO_BORDER:
        return GL_CLAMP_TO_BORDER;
    default:
        spdlog::warn("ToGLTextureWrapMode: Unknown ETextureWrapMode enum value.");
        return GL_REPEAT;
    }
}

GLenum ToGLTextureFilterMode(ETextureFilterMode mode) {
    switch (mode) {
    case ETextureFilterMode::NEAREST:
        return GL_NEAREST;
    case ETextureFilterMode::BILINEAR:
        return GL_LINEAR;
    case ETextureFilterMode::TRILINEAR:
        return GL_LINEAR_MIPMAP_LINEAR;
    default:
        spdlog::warn("ToGLTextureFilterMode: Unknown ETextureFilterMode enum value.");
        return GL_LINEAR;
    }
}

GLenum ToGLTextureFormat(ETextureFormat format) {

    switch (format) {

    case ETextureFormat::RGBA8:
        return GL_RGBA8;
    case ETextureFormat::RGB8:
        return GL_RGB8;
    case ETextureFormat::RGBA16F:
        return GL_RGBA16F;
    case ETextureFormat::RGB16F:
        return GL_RGB16F;
    case ETextureFormat::RGBA32F:
        return GL_RGBA32F;
    case ETextureFormat::RGB32F:
        return GL_RGB32F;
    case ETextureFormat::DEPTH24:
        return GL_DEPTH_COMPONENT24;
    case ETextureFormat::DEPTH32F:
        return GL_DEPTH_COMPONENT32F;
    case ETextureFormat::DEPTH24_STENCIL8:
        return GL_DEPTH24_STENCIL8;
    case ETextureFormat::DEPTH32F_STENCIL8:
        return GL_DEPTH32F_STENCIL8;
    }
    return GL_RGBA8;
}

GLTextureFormatDesc GetGLTextureFormatDesc(ETextureFormat format) {
    GLTextureFormatDesc desc{};

    desc.internalFormat = ToGLTextureFormat(format);
    desc.isDepth        = IsDepthFormat(format);

    switch (format) {
    case ETextureFormat::DEPTH24:
        desc.format  = GL_DEPTH_COMPONENT;
        desc.type    = GL_UNSIGNED_INT;
        desc.isDepth = true;
        break;

    case ETextureFormat::DEPTH32F:
        desc.format  = GL_DEPTH_COMPONENT;
        desc.type    = GL_FLOAT;
        desc.isDepth = true;
        break;

    case ETextureFormat::DEPTH24_STENCIL8:
        desc.format  = GL_DEPTH_STENCIL;
        desc.type    = GL_UNSIGNED_INT_24_8;
        desc.isDepth = true;
        break;

    case ETextureFormat::DEPTH32F_STENCIL8:
        desc.format  = GL_DEPTH_STENCIL;
        desc.type    = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
        desc.isDepth = true;
        break;

    default:
        desc.format  = GL_RGBA;
        desc.type    = GL_UNSIGNED_BYTE;
        desc.isDepth = false;
        break;
    }

    return desc;
}

GLenum ToGLTextureFormatFromChannels(int num_channels) {
    switch (num_channels) {
    case 1:
        return GL_RED;

    case 2:
        return GL_RG;
    case 3:
        return GL_RGB;

    case 4:
        return GL_RGBA;
    default:
        spdlog::warn("OpenglTexture2D::CreateInternal: Unknown number of channels: {}. Defaulting to RGBA.", num_channels);
        return GL_RGBA;
    }
}

GLenum ToGLBlendFactor(EBlendFactor factor) {
    switch (factor) {
    case EBlendFactor::BLEND_ZERO:
        return GL_ZERO;
    case EBlendFactor::BLEND_ONE:
        return GL_ONE;
    case EBlendFactor::BLEND_SRC_COLOR:
        return GL_SRC_COLOR;
    case EBlendFactor::BLEND_INV_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case EBlendFactor::BLEND_SRC_ALPHA:
        return GL_SRC_ALPHA;
    case EBlendFactor::BLEND_INV_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case EBlendFactor::BLEND_DST_ALPHA:
        return GL_DST_ALPHA;
    case EBlendFactor::BLEND_INV_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case EBlendFactor::BLEND_DST_COLOR:
        return GL_DST_COLOR;
    case EBlendFactor::BLEND_INV_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    default:
        return GL_ONE;
    }
}

GLenum ToGLBlendOp(EBlendOp op) {
    switch (op) {
    case EBlendOp::BLEND_OP_ADD:
        return GL_FUNC_ADD;
    case EBlendOp::BLEND_OP_SUBTRACT:
        return GL_FUNC_SUBTRACT;
    case EBlendOp::BLEND_OP_REV_SUBTRACT:
        return GL_FUNC_REVERSE_SUBTRACT;
    case EBlendOp::BLEND_OP_MIN:
        return GL_MIN;
    case EBlendOp::BLEND_OP_MAX:
        return GL_MAX;
    default:
        return GL_FUNC_ADD;
    }
}

GLenum ToGLComparisonFunc(EComparisonFunc func) {
    switch (func) {
    case EComparisonFunc::COMPARISON_NEVER:
        return GL_NEVER;
    case EComparisonFunc::COMPARISON_LESS:
        return GL_LESS;
    case EComparisonFunc::COMPARISON_EQUAL:
        return GL_EQUAL;
    case EComparisonFunc::COMPARISON_LESS_EQUAL:
        return GL_LEQUAL;
    case EComparisonFunc::COMPARISON_GREATER:
        return GL_GREATER;
    case EComparisonFunc::COMPARISON_NOT_EQUAL:
        return GL_NOTEQUAL;
    case EComparisonFunc::COMPARISON_GREATER_EQUAL:
        return GL_GEQUAL;
    case EComparisonFunc::COMPARISON_ALWAYS:
        return GL_ALWAYS;
    default:
        return GL_LESS;
    }
}

GLenum ToGLStencilOp(EStencilOp op) {
    switch (op) {
    case EStencilOp::STENCIL_OP_KEEP:
        return GL_KEEP;
    case EStencilOp::STENCIL_OP_ZERO:
        return GL_ZERO;
    case EStencilOp::STENCIL_OP_REPLACE:
        return GL_REPLACE;
    case EStencilOp::STENCIL_OP_INCR_SAT:
        return GL_INCR;
    case EStencilOp::STENCIL_OP_DECR_SAT:
        return GL_DECR;
    case EStencilOp::STENCIL_OP_INVERT:
        return GL_INVERT;
    case EStencilOp::STENCIL_OP_INCR:
        return GL_INCR_WRAP;
    case EStencilOp::STENCIL_OP_DECR:
        return GL_DECR_WRAP;
    default:
        return GL_KEEP;
    }
}


bool IsDepthFormat(ETextureFormat format) {
    switch (format) {
    case ETextureFormat::DEPTH24:
    case ETextureFormat::DEPTH32F:
    case ETextureFormat::DEPTH24_STENCIL8:
    case ETextureFormat::DEPTH32F_STENCIL8:
        return true;
    default:
        return false;
    }
}

GLbitfield ToGLClearFlags(EClearFlags flags) {
    GLbitfield glFlags = 0;
    if (flags & CLEAR_COLOR) {
        glFlags |= GL_COLOR_BUFFER_BIT;
    }

    if (flags & CLEAR_DEPTH) {
        glFlags |= GL_DEPTH_BUFFER_BIT;
    }

    if (flags & CLEAR_STENCIL) {
        glFlags |= GL_STENCIL_BUFFER_BIT;
    }

    return glFlags;
}

std::string GetShaderHeaderVersion() {
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    return "#version 300 es\n precision mediump float;\n";
#else
    return "#version 330 core\n";
#endif
}


GLenum ToGLCullMode(ECullMode mode) {
    switch (mode) {
    case ECullMode::CULL_MODE_FRONT:
        return GL_FRONT;
    case ECullMode::CULL_MODE_BACK:
        return GL_BACK;
    case ECullMode::CULL_MODE_FRONT_AND_BACK:
        return GL_FRONT_AND_BACK;
    default:
        return GL_BACK;
    }
}

GLenum ToGLPolygonMode(EPolygonMode mode) {
    switch (mode) {
#ifndef SDL_PLATFORM_ANDROID
    #ifndef SDL_PLATFORM_IOS
        #ifndef SDL_PLATFORM_EMSCRIPTEN
    case EPolygonMode::LINE:
        return GL_LINE;
    case EPolygonMode::POINT:
        return GL_POINT;
        #endif
    #endif
#endif
    case EPolygonMode::FILL:
    default:
        return GL_FILL;
    }
}
