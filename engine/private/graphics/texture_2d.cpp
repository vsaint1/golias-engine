#include "graphics/texture_2d.h"

#include "core/engine.h"
#include "graphics/ogl_commons.h"
#include <stb_image.h>

namespace golias {

    void Texture2D::Allocate(const TextureDesc& desc) {
        if (mTextureID) {
            glDeleteTextures(1, &mTextureID);
        }

        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        const TextureFormatGl glFormat = TextureFormatToGl(desc.Format);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     glFormat.Internal,
                     static_cast<GLsizei>(desc.Width),
                     static_cast<GLsizei>(desc.Height),
                     0,
                     glFormat.External,
                     glFormat.Type,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureMinFilterToGl(desc.Filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureMagFilterToGl(desc.Filter));

        const GLint wrap = TextureWrapToGl(desc.Wrap);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

#if defined(GOLIAS_PLATFORM_EMSCRIPTEN)
        if (desc.Wrap == TextureWrap::ClampToBorder) {
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &desc.BorderColor.x);
        }
#endif

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture2D::Texture2D(const TextureDesc& desc) : mDesc(desc) {
        mWidth    = static_cast<int32_t>(desc.Width);
        mHeight   = static_cast<int32_t>(desc.Height);
        mChannels = (desc.Format == TextureFormat::Depth24 || desc.Format == TextureFormat::Depth32F) ? 1 : 4;
        Allocate(desc);
    }

    Texture2D::Texture2D(int32_t width, int32_t height, int32_t channels, unsigned char* data)
        : mWidth(width), mHeight(height), mChannels(channels) {

        if (mWidth <= 0 || mHeight <= 0 || mChannels <= 0) {
            GOLIAS_LOG_ERROR("Invalid texture dimensions or channels. Width: %d, Height: %d, Channels: %d", mWidth, mHeight, mChannels);
            return;
        }

        GLint internalFormat = GL_RGB;
        GLenum format        = GL_RGB;

        if (mChannels == 4) {
            internalFormat = GL_RGBA;
            format         = GL_RGBA;
        } else if (mChannels == 3) {
            internalFormat = GL_RGB;
            format         = GL_RGB;
        } else if (mChannels == 1) {
            internalFormat = GL_RED;
            format         = GL_RED;
        } else {
            GOLIAS_LOG_WARN("Unsupported number of channels: %d | Using RGB", mChannels);
        }

        mDesc.Width  = static_cast<uint32_t>(mWidth);
        mDesc.Height = static_cast<uint32_t>(mHeight);
        mDesc.Format = TextureFormat::RGBA8;

        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, data);

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

    GLenum Texture2D::GetTarget() const {
        return GL_TEXTURE_2D;
    }

    const TextureDesc& Texture2D::GetDesc() const {
        return mDesc;
    }


    bool Texture2D::Recreate(const TextureDesc& desc) {
        if (desc.Width == mDesc.Width && desc.Height == mDesc.Height && desc.Format == mDesc.Format && desc.Filter == mDesc.Filter
            && desc.Wrap == mDesc.Wrap) {
            return true;
        }

        mDesc     = desc;
        mWidth    = static_cast<int32_t>(desc.Width);
        mHeight   = static_cast<int32_t>(desc.Height);
        mChannels = (desc.Format == TextureFormat::Depth24 || desc.Format == TextureFormat::Depth32F) ? 1 : 4;

        Allocate(desc);

        return true;
    }


} // namespace golias
