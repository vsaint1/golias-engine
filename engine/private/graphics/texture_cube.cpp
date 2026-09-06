#include "graphics/texture_cube.h"

#include "graphics/ogl_commons.h"

namespace golias {

    TextureCube::TextureCube(const TextureDesc& desc) {
        Recreate(desc);
    }

    TextureCube::~TextureCube() {
        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }
    }

    bool TextureCube::Recreate(const TextureDesc& desc) {

        if (!desc.Width || !desc.Height || desc.Width != desc.Height) {
            return false;
        }

        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }

        mDesc = desc;
        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);

        const TextureFormatGl glFormat = TextureFormatToGl(desc.Format);

        for (int face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, glFormat.Internal, desc.Width, desc.Height, 0, glFormat.External,
                         glFormat.Type, nullptr);
        }

        const GLint filter = TextureMinFilterToGl(desc.Filter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureMagFilterToGl(desc.Filter));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        return true;
    }

    GLuint TextureCube::GetHandle() const {
        return mTextureID;
    }

    GLenum TextureCube::GetTarget() const {
        return GL_TEXTURE_CUBE_MAP;
    }

    const TextureDesc& TextureCube::GetDesc() const {
        return mDesc;
    }

} // namespace golias
