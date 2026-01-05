#include "core/graphics/texture_2d.h"

#include "core/engine.h"

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


    std::shared_ptr<Texture2D> Texture2D::Load(const std::string_view pFilePath) {
        auto tManager = golias::Engine::GetInstance().GetTextureManager();
        return tManager.EnsureTexture2D(pFilePath);
    }

    std::shared_ptr<Texture2D> TextureManager::EnsureTexture2D(const std::string_view pFilePath) {
        auto it = textures2D.find(pFilePath.data());

        if (it != textures2D.end()) {
            // spdlog::debug("Texture cache HIT: {} (Total cached: {})", pFilePath, textures2D.size());
            return it->second;
        }

        auto rd = golias::Engine::GetInstance().GetRenderingDevice();

        auto texture = rd->CreateTextureFromFile(pFilePath);

        if (texture) {
            textures2D[pFilePath.data()] = texture;
            spdlog::debug("Texture cache MISS, loaded: {} (Total cached: {})", pFilePath, textures2D.size());
        }

        return texture;
    }

    std::shared_ptr<Texture2D> TextureManager::EnsureTexture2D(const std::string_view pFilePath,int width, int height, ETextureFormat format, const Uint8* data) {

        auto it = textures2D.find(pFilePath.data());

        if (it != textures2D.end()) {
            spdlog::debug("Texture cache HIT (embedded): {} (Total cached: {})", pFilePath, textures2D.size());
            return it->second;
        }

        auto rd = golias::Engine::GetInstance().GetRenderingDevice();

        auto texture = rd->CreateTextureFromData(width, height, format, data);

        if (texture) {
            textures2D[pFilePath.data()] = texture;
            spdlog::debug("Texture cache MISS (embedded), created: {} {}x{} (Total cached: {})", 
                         pFilePath, width, height, textures2D.size());
        }

        return texture;
    }

} // namespace golias
