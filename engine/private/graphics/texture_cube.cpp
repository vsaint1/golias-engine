#include "graphics/texture_cube.h"

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

        for (int face = 0; face < 6; ++face) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA8, desc.Width, desc.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, desc.Filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, desc.Filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST);
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
