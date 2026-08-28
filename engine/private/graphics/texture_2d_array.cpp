#include "graphics/texture_2d_array.h"

namespace golias {


    //  TODO: move this to ogl_commons.h for example 
    static void texture_format(TextureFormat format, GLint& internal, GLenum& external, GLenum& type) {
        internal = format == TextureFormat::Depth32F ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT24;
        external = GL_DEPTH_COMPONENT;
        type     = GL_FLOAT;
        
        if (format == TextureFormat::RGBA8) {
            internal = GL_RGBA8;
            external = GL_RGBA;
            type     = GL_UNSIGNED_BYTE;
        }

    }

    Texture2DArray::Texture2DArray(const TextureDesc& desc) {
        Recreate(desc);
    }

    Texture2DArray::~Texture2DArray() {
        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }
    }

    bool Texture2DArray::Recreate(const TextureDesc& desc) {
        
        if (!desc.Width || !desc.Height || !desc.Layers) {
            return false;
        }

        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }

        mDesc = desc;
        GLint internal;
        GLenum external, type;
        texture_format(desc.Format, internal, external, type);

        glGenTextures(1, &mTextureID);

        glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internal, desc.Width, desc.Height, desc.Layers, 0, external, type, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, desc.Filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, desc.Filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST);

        const GLint wrap = desc.Wrap == TextureWrap::Repeat        ? GL_REPEAT
                         : desc.Wrap == TextureWrap::ClampToBorder ? GL_CLAMP_TO_BORDER
                                                                   : GL_CLAMP_TO_EDGE;

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, wrap);
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, &desc.BorderColor.x);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        return true;
    }

    GLuint Texture2DArray::GetHandle() const {
        return mTextureID;
    }

    GLenum Texture2DArray::GetTarget() const {
        return GL_TEXTURE_2D_ARRAY;
    }

    const TextureDesc& Texture2DArray::GetDesc() const {
        return mDesc;
    }

} // namespace golias
