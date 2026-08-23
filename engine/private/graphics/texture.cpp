#include "graphics/texture.h"

#include "core/engine.h"
#include <stb_image.h>

namespace golias {

    Texture2D::Texture2D(int32_t width, int32_t height, int32_t channels, unsigned char* data)
        : mWidth(width), mHeight(height), mChannels(channels) {

        if (mWidth <= 0 || mHeight <= 0 || mChannels <= 0) {
            GOLIAS_LOG_ERROR("Invalid texture dimensions or channels. Width: %d, Height: %d, Channels: %d", mWidth, mHeight, mChannels);
            return;
        }

        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        GOLIAS_LOG_INFO(
            "Texture created successfully. Width: %d, Height: %d, Channels: %d, Handle: %u", mWidth, mHeight, mChannels, mTextureID);
    }

    Ref<Texture2D> Texture2D::Load(CString path) {

        int w, h, c;

        FileSystem& fileSystem = Engine::GetInstance().GetFileSystem();

        Path fullPath = fileSystem.GetAssetsFolder() / path.data();

        if (!fileSystem.FileExists(fullPath)) {
            GOLIAS_LOG_ERROR("Texture file does not exist: %s", fullPath.string().c_str());
            return nullptr;
        }


        unsigned char* data = stbi_load(fullPath.string().c_str(), &w, &h, &c, 0);

        Ref<Texture2D> tex;

        if (data) {
            tex = std::make_shared<Texture2D>(w, h, c, data);
            stbi_image_free(data);
        } else {
            GOLIAS_LOG_ERROR("Failed to load texture: %s", fullPath.string().c_str());
        }


        return tex;
    }


    Texture2D::~Texture2D() {
        if (mTextureID != 0) {
            glDeleteTextures(1, &mTextureID);
        }
    }


    int32_t Texture2D::GetWidth() const {
        return mWidth;
    }

    int32_t Texture2D::GetHeight() const {
        return mHeight;
    }

    int32_t Texture2D::GetChannels() const {
        return mChannels;
    }

    GLuint Texture2D::GetHandle() const {
        return mTextureID;
    }
} // namespace golias
