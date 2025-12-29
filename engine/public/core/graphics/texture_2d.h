#pragma once

#include "structs.h"
#include <memory>
#include <unordered_map>

namespace golias {


    class Texture2D {
    public:
        virtual ~Texture2D() = default;

        Uint32 GetWidth() const;

        Uint32 GetHeight() const;

        Uint32 GetNativeHandle() const;

        Uint32 GetNumChannels() const;

        ETextureFormat GetFormat() const;

        const std::string& GetFilePath() const;

        bool IsValid() const;

    protected:
        Uint32 handle   = 0;
        Uint32 width    = 0;
        Uint32 height   = 0;
        Uint32 channels = 0;

        std::string file_path;

        ETextureFormat format                            = ETextureFormat::RGBA;
        ETextureWrapMode wrap_mode                       = ETextureWrapMode::REPEAT;
        ETextureFilterMode filter_mode                   = ETextureFilterMode::BILINEAR;
        ETextureMipGenSettings mip_gen_settings          = ETextureMipGenSettings::SIMPLE_AVERAGE;
        ETextureWrapMode address_mode                    = ETextureWrapMode::REPEAT;
        ETextureCompressionSettings compression_settings = ETextureCompressionSettings::DEFAULT;

        bool bSRGB = false;

        Texture2D() = default;

        Texture2D(const std::string_view pFilePath) : file_path(pFilePath) {
        }

        Texture2D(int w, int h, ETextureFormat fmt, Uint8* data) : width(w), height(h), format(fmt){}
    };

    class TextureManager2D {
    public:
        std::shared_ptr<Texture2D> EnsureTexture(const std::string_view pFilePath);
        std::shared_ptr<Texture2D> EnsureTexture(const std::string_view pFilePath,int width, int height, ETextureFormat format, const Uint8* data);

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures;
    };

} // namespace golias
