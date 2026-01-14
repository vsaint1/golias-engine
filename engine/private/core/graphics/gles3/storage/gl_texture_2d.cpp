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

        switch (fmt) {
        case ETextureFormat::RED:
            channels = 1;
            break;
        case ETextureFormat::RG:
            channels = 2;
            break;
        case ETextureFormat::RGB:
            channels = 3;
            break;
        case ETextureFormat::RGBA:
            channels = 4;
            break;
        default:
            channels = 4;
            spdlog::warn("OpenglTexture2D::OpenglTexture2D Unknown ETextureFormat, defaulting to 4 channels (RGBA).");
            break;
        }

        CreateInternal(data);
    }


    OpenglTexture2D::OpenglTexture2D(Uint32 width, Uint32 height, EFramebufferTextureFormat format) {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);

        GLenum internalFormat = FramebufferTextureFormatToGL(format);
        GLenum glFormat       = GL_RGBA;
        GLenum type           = GL_UNSIGNED_BYTE;

        bool depth = false;

        switch (format) {
        case EFramebufferTextureFormat::DEPTH24:
        case EFramebufferTextureFormat::DEPTH32F:
            glFormat = GL_DEPTH_COMPONENT;
            type     = GL_FLOAT;
            depth    = true;
            break;

        case EFramebufferTextureFormat::DEPTH24_STENCIL8:
        case EFramebufferTextureFormat::DEPTH32F_STENCIL8:
            glFormat = GL_DEPTH_STENCIL;
            type     = GL_UNSIGNED_INT_24_8;
            depth    = true;
            break;

        default:
            glFormat = GL_RGBA;
            type     = GL_UNSIGNED_BYTE;
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, nullptr);

        if (depth) {
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

        stbi_image_free(data);
    }

    OpenglTexture2D::~OpenglTexture2D() {
        if (handle) {
            glDeleteTextures(1, &handle);
        }
    }


} // namespace golias
