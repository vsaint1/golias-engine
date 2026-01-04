#include "core/graphics/gles3/gl_common.h"

#include <spdlog/spdlog.h>


GLint ToGLBuferTarget(EBufferTarget target) {
    switch (target) {
    case EBufferTarget::ARRAY_BUFFER:
        return GL_ARRAY_BUFFER;
    case EBufferTarget::ELEMENT_ARRAY_BUFFER:
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
    case ETextureFormat::RED:
        return GL_RED;
    case ETextureFormat::RG:
        return GL_RG;
    case ETextureFormat::RGB:
        return GL_RGB;
    case ETextureFormat::RGBA:
        return GL_RGBA;
    default:
        spdlog::warn("ToGLTextureFormat: Unknown ETextureFormat enum value.");

        return GL_RGBA;
    }
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



std::string GetShaderHeaderVersion() {
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
        return "#version 300 es\n";
#else
        return "#version 330 core\n";
#endif
    }