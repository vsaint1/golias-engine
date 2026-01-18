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

        spdlog::info("OpenglTexture2D::OpenglTexture2D Loaded texture from file: {} (Handle: {}, Width: {}, Height: {}, Channels: {})",
                     pFilePath,
                     handle,
                     width,
                     height,
                     channels);
    }

    OpenglTexture2D::OpenglTexture2D(int w, int h, ETextureFormat fmt, Uint8* data) : Texture2D(w, h, fmt, data) {

        channels = ChannelsFromTextureFormat(fmt);

        CreateInternal(data);
    }


    OpenglTexture2D::OpenglTexture2D(Uint32 width, Uint32 height, ETextureFormat format) {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);

        auto gl_fmt = GetGLTextureFormatDesc(format);

        glTexImage2D(GL_TEXTURE_2D, 0, gl_fmt.internalFormat, width, height, 0, gl_fmt.format, gl_fmt.type, nullptr);

        if (gl_fmt.isDepth) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            float border[4] = {1, 1, 1, 1};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }


    void OpenglTexture2D::CreateInternal(Uint8* data) {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);


        GLenum gl_fmt = ToGLTextureFormatFromChannels(static_cast<int>(channels));

        glTexImage2D(GL_TEXTURE_2D, 0, gl_fmt, width, height, 0, gl_fmt, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SDL_free(data);
    }

    OpenglTexture2D::~OpenglTexture2D() {
        if (handle) {
            glDeleteTextures(1, &handle);
        }
    }


} // namespace golias
