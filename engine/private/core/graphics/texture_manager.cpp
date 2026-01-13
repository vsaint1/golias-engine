#include "core/graphics/texture_manager.h"
#include "core/graphics/texture_2d.h"
#include "core/graphics/texture_cubemap.h"
#include "core/engine.h"

namespace golias {
        std::shared_ptr<Texture2D> TextureManager::EnsureTexture2D(const std::string_view pFilePath) {
        auto it = textures2D.find(pFilePath.data());

        if (it != textures2D.end()) {
            spdlog::debug("Texture cache HIT: {} (Total cached: {})", pFilePath, textures2D.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateTextureFromFile(pFilePath);

        if (texture) {
            textures2D[pFilePath.data()] = texture;
            spdlog::debug("Texture cache MISS, loaded: {} (Total cached: {})", pFilePath, textures2D.size());
        }

        return texture;
    }

    std::shared_ptr<Texture2D>
        TextureManager::EnsureTexture2D(const std::string_view pFilePath, int width, int height, ETextureFormat format, const Uint8* data) {

        auto it = textures2D.find(pFilePath.data());

        if (it != textures2D.end()) {
            spdlog::debug("Texture cache HIT (embedded): {} (Total cached: {})", pFilePath, textures2D.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateTextureFromData(width, height, format, data);

        if (texture) {
            textures2D[pFilePath.data()] = texture;
            spdlog::debug(
                "Texture cache MISS (embedded), created: {} {}x{} (Total cached: {})", pFilePath, width, height, textures2D.size());
        }

        return texture;
    }

    std::shared_ptr<TextureCubemap> TextureManager::EnsureTextureCubemapCross(const std::string_view pFilePath) {

        auto it = texturesCubemap.find(pFilePath.data());
        if (it != texturesCubemap.end()) {
            spdlog::debug("Cubemap cache HIT (cross): {} (Total cached: {})", pFilePath, texturesCubemap.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateCubemapFromCross(pFilePath.data());

        if (texture) {
            texturesCubemap[pFilePath.data()] = texture;
            spdlog::debug("Cubemap cache MISS (cross), loaded: {} (Total cached: {})", pFilePath, texturesCubemap.size());
        }

        return texture;
    }

    std::shared_ptr<TextureCubemap> TextureManager::EnsureTextureCubemapFaces(const std::array<std::string, 6>& faces) {
        auto it = texturesCubemap.find(faces.back());
        if (it != texturesCubemap.end()) {
            spdlog::debug("Cubemap cache HIT (faces): {} (Total cached: {})", faces[0], texturesCubemap.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateCubemapFromFaces(faces);

        if (texture) {
            texturesCubemap[faces[0]] = texture;
            spdlog::debug("Cubemap cache MISS (faces), loaded: {} (Total cached: {})", faces[0], texturesCubemap.size());
        }

        return texture;
    }
}