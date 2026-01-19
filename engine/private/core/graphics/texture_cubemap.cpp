#include "core/graphics/texture_cubemap.h"

#include "core/engine.h"

namespace golias {
    Uint32 TextureCubemap::GetWidth() const {
        return width;
    }

    Uint32 TextureCubemap::GetHeight() const {
        return height;
    }

    Uint32 TextureCubemap::GetNativeHandle() const {
        return handle;
    }

    Uint32 TextureCubemap::GetNumChannels() const {
        return channels;
    }

    ETextureFormat TextureCubemap::GetFormat() const {
        return format;
    }

    bool TextureCubemap::IsValid() const {
        return handle != 0;
    }


    std::shared_ptr<TextureCubemap> TextureCubemap::Load(const std::string_view pFilePath) {
        auto& assetManager = Engine::GetInstance().GetAssetManager();
        auto cubemap  = assetManager.EnsureTextureCubemapCross(pFilePath);

        return cubemap;
    }


    std::shared_ptr<TextureCubemap> TextureCubemap::Load(const std::array<std::string, 6>& faces) {
        auto& assetManager = Engine::GetInstance().GetAssetManager();
        auto cubemap  = assetManager.EnsureTextureCubemapFaces(faces);

        return cubemap;
    }


    std::shared_ptr<TextureCubemap> TextureCubemap::LoadProcedural() {
        auto& assetManager = Engine::GetInstance().GetAssetManager();
        auto cubemap  = assetManager.EnsureTextureCubemapProcedural();

        return cubemap;
    }

    glm::vec3 TextureCubemap::GetCubemapDirection(int face, float u, float v) {
        switch (face) {
        case 0:
            return glm::vec3(1.0f, -v, -u); // +X (right)
        case 1:
            return glm::vec3(-1.0f, -v, u); // -X (left)
        case 2:
            return glm::vec3(u, 1.0f, v); // +Y (top)
        case 3:
            return glm::vec3(u, -1.0f, -v); // -Y (bottom)
        case 4:
            return glm::vec3(u, -v, 1.0f); // +Z (front)
        case 5:
            return glm::vec3(-u, -v, -1.0f); // -Z (back)
        default:
            return glm::vec3(0.0f);
        }
    }

    glm::vec2 TextureCubemap::DirectionToEquirectangularUV(const glm::vec3& dir) {
        float theta = atan2f(dir.z, dir.x);
        float phi   = asinf(dir.y);
        float u     = 0.5f + 0.5f * theta / glm::pi<float>();
        float v     = 0.5f - phi / glm::pi<float>();
        return glm::vec2(fmodf(u, 1.0f), glm::clamp(v, 0.0f, 1.0f));
    }

} // namespace golias
