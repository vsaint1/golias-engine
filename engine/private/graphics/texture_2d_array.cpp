#include "graphics/texture_2d_array.h"

#include "graphics/ogl_commons.h"

namespace golias {

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

        mDesc                          = desc;
        const TextureFormatGl glFormat = TextureFormatToGl(desc.Format);

        glGenTextures(1, &mTextureID);

        glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);
        glTexImage3D(
            GL_TEXTURE_2D_ARRAY, 0, glFormat.Internal, desc.Width, desc.Height, desc.Layers, 0, glFormat.External, glFormat.Type, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, TextureMinFilterToGl(desc.Filter));
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, TextureMagFilterToGl(desc.Filter));

        const GLint wrap = TextureWrapToGl(desc.Wrap);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, wrap);

#if defined(GOLIAS_PLATFORM_EMSCRIPTEN)
        if (desc.Wrap == TextureWrap::ClampToBorder) {
            glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, &desc.BorderColor.x);
        }
#endif

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
