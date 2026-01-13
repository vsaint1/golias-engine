#pragma once
#include "core/graphics/structs.h"
#include <memory>

namespace golias {

    class Texture2D;
    class TextureCubemap;

    class TextureManager {
    public:
        std::shared_ptr<Texture2D> EnsureTexture2D(const std::string_view pFilePath);
        std::shared_ptr<Texture2D>
            EnsureTexture2D(const std::string_view pFilePath, int width, int height, ETextureFormat format, const Uint8* data);

        std::shared_ptr<TextureCubemap> EnsureTextureCubemapCross(const std::string_view pFilePath);
        std::shared_ptr<TextureCubemap> EnsureTextureCubemapFaces(const std::array<std::string, 6>& faces);

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures2D;
        std::unordered_map<std::string, std::shared_ptr<TextureCubemap>> texturesCubemap;
    };
} // namespace golias
