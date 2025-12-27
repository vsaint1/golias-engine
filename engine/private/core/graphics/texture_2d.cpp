#include "core/graphics/texture_2d.h"



namespace golias {

    Uint32 Texture2D::GetWidth() const {
        return width;
    }

    Uint32 Texture2D::GetHeight() const {
        return height;
    }

    Uint32 Texture2D::GetNativeHandle() const {
        return handle;
    }

    Uint32 Texture2D::GetNumChannels() const {
        return channels;
    }

    ETextureFormat Texture2D::GetFormat() const {
        return format;
    }

    const std::string& Texture2D::GetFilePath() const {
        return file_path;
    }

    bool Texture2D::IsValid() const {
        return handle != 0;
    }
} // namespace golias
