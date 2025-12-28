#include "core/graphics/gles3/storage/gl_texture_2d.h"

#include "core/graphics/gles3/gl_common.h"
#include <spdlog/spdlog.h>

namespace golias {

    OpenglTexture2D::OpenglTexture2D(const std::string_view pFilePath) : Texture2D(pFilePath) {
        int w, h, n;
        stbi_set_flip_vertically_on_load(false);

        Uint8* data = stbi_load(pFilePath.data(), &w, &h, &n, 0);

        if (!data) {
            spdlog::error("OpenglTexture2D::OpenglTexture2D Failed to load Texture from file: {}", pFilePath);
            return;
        }

        width    = static_cast<Uint32>(w);
        height   = static_cast<Uint32>(h);
        channels = static_cast<Uint32>(n);

        CreateInternal(data);

        spdlog::info("OpenglTexture2D::OpenglTexture2D Loaded texture from file: {} (Handle: {}, Width: {}, Height: {}, Channels: {})", pFilePath,
                     handle, width, height, channels);
    }

    OpenglTexture2D::OpenglTexture2D(int w, int h, int num_channels, Uint8* data) : Texture2D(w, h, num_channels) {
        CreateInternal(data);
    }

    void OpenglTexture2D::CreateInternal(Uint8* data) {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);

        GLenum format = ToGLTextureFormatFromChannels(channels);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    OpenglTexture2D::~OpenglTexture2D() {
        if (handle) {
            glDeleteTextures(1, &handle);
        }
    }


} // namespace golias
