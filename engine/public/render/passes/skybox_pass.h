#pragma once

#include "render/render_pass.h"
#include "render/shader_parameter.h"

namespace golias {

    class Mesh;
    class Shader;
    class TextureCube;
    class Texture2D;

    struct SkySettings {
        glm::vec3 SkyTint         = glm::vec3(0.22f, 0.50f, 1.0f);
        glm::vec3 GroundColor     = glm::vec3(0.16f, 0.08f, 0.035f);
        glm::vec3 SunDirection    = glm::normalize(glm::vec3(0.0f, 0.35f, 0.94f));
        float AtmosphereThickness = 1.15f;
        float Exposure            = 1.0f;
        float TextureAngularSize  = 0.045f;
        float BrightnessFactor    = 8.0f; // 8-16
        float SunBrightness       = 0.5f;
        float StarIntensity       = 0.6f;
        float TimeOfDay           = 0.0f;

        void ApplyTimeOfDay(float hour);
    };

    /// @brief  Renders the skybox using the specified sky settings.
    class SkyboxPass : public RenderPass {
    public:
        ~SkyboxPass() override;

        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        void SetSettings(const SkySettings& settings) {
            mSettings = settings;
        }

        const char* GetName() const override {
            return "SkyboxPass";
        }

    private:
        Ref<Shader> mShader          = nullptr;
        Ref<TextureCube> mSkyCubemap = nullptr;

        Ref<Texture2D> mSunTexture  = nullptr;
        Ref<Texture2D> mMoonTexture = nullptr;

        Mesh* mSkyMesh = nullptr;
        SkyboxParameter mSkyboxParams;
        SkySettings mSettings;
    };

} // namespace golias
