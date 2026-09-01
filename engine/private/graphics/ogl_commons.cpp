#include "graphics/ogl_commons.h"

namespace golias {

    TextureFormatGl TextureFormatToGl(TextureFormat format) {
        TextureFormatGl result{};
        switch (format) {
        case TextureFormat::RGBA8:
            result.Internal = GL_RGBA8;
            result.External = GL_RGBA;
            result.Type     = GL_UNSIGNED_BYTE;
            break;
        case TextureFormat::RGBA16F:
            result.Internal = GL_RGBA16F;
            result.External = GL_RGBA;
            result.Type     = GL_HALF_FLOAT;
            break;
        case TextureFormat::Depth24:
            result.Internal = GL_DEPTH_COMPONENT24;
            result.External = GL_DEPTH_COMPONENT;
            result.Type     = GL_FLOAT;
            break;
        case TextureFormat::Depth32F:
            result.Internal = GL_DEPTH_COMPONENT32F;
            result.External = GL_DEPTH_COMPONENT;
            result.Type     = GL_FLOAT;
            break;
        }
        return result;
    }

    GLint TextureMinFilterToGl(TextureFilter filter) {
        return filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST;
    }

    GLint TextureMagFilterToGl(TextureFilter filter) {
        return filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST;
    }

    GLint TextureWrapToGl(TextureWrap wrap) {
        switch (wrap) {
        case TextureWrap::Repeat:
            return GL_REPEAT;
        case TextureWrap::ClampToBorder:
            return GL_CLAMP_TO_BORDER;
        case TextureWrap::ClampToEdge:
        default:
            return GL_CLAMP_TO_EDGE;
        }
    }

    GLenum BufferTargetToGl(BufferTarget target) {
        switch (target) {
        case BufferTarget::Vertex:
            return GL_ARRAY_BUFFER;
        case BufferTarget::Index:
            return GL_ELEMENT_ARRAY_BUFFER;
        case BufferTarget::Uniform:
            return GL_UNIFORM_BUFFER;
        default:
            return GL_ARRAY_BUFFER;
        }
    }

} // namespace golias
