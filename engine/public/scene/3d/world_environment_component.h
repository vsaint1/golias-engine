#pragma once
#include "scene/component.h"

#include <glm/vec4.hpp>

namespace golias {

    class TextureCubemap;
    class Mesh;

    enum EToneMappingMode {
        TONE_MAPPING_LINEAR     = 0,
        TONE_MAPPING_REINHARD   = 1,
        TONE_MAPPING_FILMIC     = 2,
        TONE_MAPPING_ACES       = 3,
        TONE_MAPPING_UNCHARTED2 = 4,
    };

    enum EWorldEnvironmentMode {
        WORLD_ENVIRONMENT_MODE_CLEAR_COLOR  = 0,
        WORLD_ENVIRONMENT_MODE_CUSTOM_COLOR = 1,
        WORLD_ENVIRONMENT_MODE_SKYBOX       = 2,
    };

    class WorldEnvironmentComponent : public Component {
        COMPONENT(WorldEnvironmentComponent)
    public:
        WorldEnvironmentComponent() = default;
        WorldEnvironmentComponent(const std::shared_ptr<TextureCubemap>& cubemap, EToneMappingMode mode, float exposure)
            : texture(cubemap), toneMappingMode(mode), exposure_(exposure) {
        }

        ~WorldEnvironmentComponent() override = default;

        void Start() override;

        void Update(float deltaTime) override;

        void SetSkyboxCubemap(const std::shared_ptr<TextureCubemap>& cubemap);

        std::shared_ptr<TextureCubemap> GetTextureCubemap() const;

        EToneMappingMode GetToneMappingMode() const;
        void SetToneMappingMode(EToneMappingMode mode);

        float GetExposure() const;
        void SetExposure(float exp);

        glm::vec4 GetClearColor() const;
        void SetClearColor(const glm::vec4& color);

        EWorldEnvironmentMode GetEnvironmentMode() const;

        void SetEnvironmentMode(EWorldEnvironmentMode mode);

        std::shared_ptr<Mesh> GetSkyboxMesh() const;

    private:
        std::shared_ptr<TextureCubemap> texture = nullptr;
        EToneMappingMode toneMappingMode        = EToneMappingMode::TONE_MAPPING_ACES;
        EWorldEnvironmentMode environmentMode   = EWorldEnvironmentMode::WORLD_ENVIRONMENT_MODE_CLEAR_COLOR;
        std::shared_ptr<Mesh> skybox_mesh       = nullptr;
        float exposure_                         = 0.5f;
        glm::vec4 clearColor                    = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    };
}; // namespace golias
