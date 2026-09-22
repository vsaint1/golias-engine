#pragma once

#include "component.h"
#include "graphics/render_types.h"

namespace golias {

    enum class ParticleShape : uint8_t {
        Quad,
        Line,
        Mesh,
    };

    struct ParticleEmitterSettings {
        ParticleShape Shape = ParticleShape::Quad;

        uint32_t MaxParticles  = 128;
        float EmissionRate     = 24.0f;
        float Lifetime         = 2.0f;
        float LifetimeVariance = 0.0f;

        glm::vec3 Position         = glm::vec3(0.0f);
        glm::vec3 PositionVariance = glm::vec3(0.0f);
        glm::vec3 Velocity         = glm::vec3(0.0f);
        glm::vec3 VelocityVariance = glm::vec3(0.0f);
        glm::vec3 Acceleration     = glm::vec3(0.0f);

        glm::vec2 StartSize  = glm::vec2(0.1f);
        glm::vec2 EndSize    = glm::vec2(0.1f);
        glm::vec4 StartColor = glm::vec4(1.0f);
        glm::vec4 EndColor   = glm::vec4(1.0f);

        float LineLength   = 0.5f;
        float SizeVariance = 0.0f;
        bool Loop          = true;
        bool Enabled       = true;
    };

    class Material;
    class Mesh;

    class CpuParticleComponent : public Component {
        COMPONENT(CpuParticleComponent)

    public:
        CpuParticleComponent() = default;
        explicit CpuParticleComponent(const ParticleEmitterSettings& settings);

        bool LoadProperties(const Json& properties) override;
        bool SaveProperties(Json& properties) const override;

        void Start() override;

        void Update(float deltaTime) override;

        void OnEnable() override;
        void OnDisable() override;

        void SetSettings(const ParticleEmitterSettings& settings);
        const ParticleEmitterSettings& GetSettings() const;

        void SetMaterial(const Ref<Material>& material);
        Ref<Material> GetMaterial() const;

        void Play();
        void Stop();

        void Restart();
        bool IsPlaying() const;

    private:
        struct Particle {
            glm::vec3 Position = glm::vec3(0.0f);
            glm::vec3 Velocity = glm::vec3(0.0f);
            float Age          = 0.0f;
            float Lifetime     = 1.0f;
            float SizeScale    = 1.0f;
        };

        void Emit();
        void SubmitRenderCommand() const;

        ParticleEmitterSettings mSettings;

        Ref<Material> mMaterial = nullptr;
        String mMaterialPath;
        
        Ref<Mesh> mMesh = nullptr;


        std::vector<Particle> mParticles = {};

        float mEmissionAccumulator       = 0.0f;
        bool mPlaying                    = true;
    };

} // namespace golias
