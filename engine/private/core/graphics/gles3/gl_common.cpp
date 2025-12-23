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
