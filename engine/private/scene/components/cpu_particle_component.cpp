#include "scene/components/cpu_particle_component.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "math/random.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/game_object.h"

namespace golias {

    constexpr size_t kFloatsPerVertex = 9;


    CpuParticleComponent::CpuParticleComponent(const ParticleEmitterSettings& settings) : mSettings(settings) {
        mPlaying = settings.Enabled;
    }

    bool CpuParticleComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        mSettings.MaxParticles     = properties.value("max_particles", mSettings.MaxParticles);
        mSettings.EmissionRate     = properties.value("emission_rate", mSettings.EmissionRate);
        mSettings.Lifetime         = properties.value("lifetime", mSettings.Lifetime);
        mSettings.LifetimeVariance = properties.value("lifetime_variance", mSettings.LifetimeVariance);
        mSettings.LineLength       = properties.value("line_length", mSettings.LineLength);
        mSettings.SizeVariance     = properties.value("size_variance", mSettings.SizeVariance);
        mSettings.Loop             = properties.value("loop", mSettings.Loop);
        mSettings.Enabled          = properties.value("playing", mSettings.Enabled);

        const String shape = properties.value("shape", "quad");
        if (shape == "line") {
            mSettings.Shape = ParticleShape::Line;
        } else if (shape == "mesh") {
            mSettings.Shape = ParticleShape::Mesh;
        } else {
            mSettings.Shape = ParticleShape::Quad;
        }

        if (mSettings.Shape == ParticleShape::Mesh && properties.contains("mesh") && properties["mesh"].is_object()) {
            mMesh = Mesh::CreateFromJson(properties["mesh"]);
        } else if (mSettings.Shape == ParticleShape::Quad) {
            mMesh = Mesh::CreateQuad();
        } else if (mSettings.Shape == ParticleShape::Line) {
            mMesh = Mesh::CreateCube(glm::vec3(0.02f, 1.0f, 0.02f));
        }

        if (properties.contains("position") && properties["position"].is_object()) {
            const Json& position = properties["position"];
            mSettings.Position.x = position.value("x", mSettings.Position.x);
            mSettings.Position.y = position.value("y", mSettings.Position.y);
            mSettings.Position.z = position.value("z", mSettings.Position.z);
        }

        if (properties.contains("position_variance") && properties["position_variance"].is_object()) {
            const Json& variance         = properties["position_variance"];
            mSettings.PositionVariance.x = variance.value("x", mSettings.PositionVariance.x);
            mSettings.PositionVariance.y = variance.value("y", mSettings.PositionVariance.y);
            mSettings.PositionVariance.z = variance.value("z", mSettings.PositionVariance.z);
        }

        if (properties.contains("velocity") && properties["velocity"].is_object()) {
            const Json& velocity = properties["velocity"];
            mSettings.Velocity.x = velocity.value("x", mSettings.Velocity.x);
            mSettings.Velocity.y = velocity.value("y", mSettings.Velocity.y);
            mSettings.Velocity.z = velocity.value("z", mSettings.Velocity.z);
        }

        if (properties.contains("velocity_variance") && properties["velocity_variance"].is_object()) {
            const Json& variance         = properties["velocity_variance"];
            mSettings.VelocityVariance.x = variance.value("x", mSettings.VelocityVariance.x);
            mSettings.VelocityVariance.y = variance.value("y", mSettings.VelocityVariance.y);
            mSettings.VelocityVariance.z = variance.value("z", mSettings.VelocityVariance.z);
        }

        if (properties.contains("acceleration") && properties["acceleration"].is_object()) {
            const Json& acceleration = properties["acceleration"];
            mSettings.Acceleration.x = acceleration.value("x", mSettings.Acceleration.x);
            mSettings.Acceleration.y = acceleration.value("y", mSettings.Acceleration.y);
            mSettings.Acceleration.z = acceleration.value("z", mSettings.Acceleration.z);
        }

        if (properties.contains("start_size") && properties["start_size"].is_object()) {
            const Json& size      = properties["start_size"];
            mSettings.StartSize.x = size.value("x", mSettings.StartSize.x);
            mSettings.StartSize.y = size.value("y", mSettings.StartSize.y);
        }

        if (properties.contains("end_size") && properties["end_size"].is_object()) {
            const Json& size    = properties["end_size"];
            mSettings.EndSize.x = size.value("x", mSettings.EndSize.x);
            mSettings.EndSize.y = size.value("y", mSettings.EndSize.y);
        }

        if (properties.contains("start_color") && properties["start_color"].is_object()) {
            const Json& color      = properties["start_color"];
            mSettings.StartColor.x = color.value("x", mSettings.StartColor.x);
            mSettings.StartColor.y = color.value("y", mSettings.StartColor.y);
            mSettings.StartColor.z = color.value("z", mSettings.StartColor.z);
            mSettings.StartColor.w = color.value("w", mSettings.StartColor.w);
        }

        if (properties.contains("end_color") && properties["end_color"].is_object()) {
            const Json& color    = properties["end_color"];
            mSettings.EndColor.x = color.value("x", mSettings.EndColor.x);
            mSettings.EndColor.y = color.value("y", mSettings.EndColor.y);
            mSettings.EndColor.z = color.value("z", mSettings.EndColor.z);
            mSettings.EndColor.w = color.value("w", mSettings.EndColor.w);
        }

        if (properties.contains("material") && properties["material"].is_object()) {
            mMaterialPath = properties["material"].value("path", "");
            mMaterial     = Material::LoadFromJson(properties["material"]);
        }

        if (mMaterial && mMaterial->GetShader()) {
            mMaterial->GetShader()->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        }

        mParticles.clear();
        mEmissionAccumulator = 0.0f;
        mPlaying             = mSettings.Enabled;
        return true;
    }

    bool CpuParticleComponent::SaveProperties(Json& properties) const {
        Component::SaveProperties(properties);

        properties["max_particles"]     = mSettings.MaxParticles;
        properties["emission_rate"]     = mSettings.EmissionRate;
        properties["lifetime"]          = mSettings.Lifetime;
        properties["lifetime_variance"] = mSettings.LifetimeVariance;
        properties["shape"] = mSettings.Shape == ParticleShape::Line ? "line" : mSettings.Shape == ParticleShape::Mesh ? "mesh" : "quad";
        properties["line_length"]   = mSettings.LineLength;
        properties["size_variance"] = mSettings.SizeVariance;
        properties["loop"]          = mSettings.Loop;
        properties["playing"]       = mSettings.Enabled;

        properties["position"] = {
            {"x", mSettings.Position.x},
            {"y", mSettings.Position.y},
            {"z", mSettings.Position.z}
        };

        properties["position_variance"] = {
            {"x", mSettings.PositionVariance.x},
            {"y", mSettings.PositionVariance.y},
            {"z", mSettings.PositionVariance.z}
        };

        properties["velocity"] = {
            {"x", mSettings.Velocity.x},
            {"y", mSettings.Velocity.y},
            {"z", mSettings.Velocity.z}
        };

        properties["velocity_variance"] = {
            {"x", mSettings.VelocityVariance.x},
            {"y", mSettings.VelocityVariance.y},
            {"z", mSettings.VelocityVariance.z}
        };
        properties["acceleration"] = {
            {"x", mSettings.Acceleration.x},
            {"y", mSettings.Acceleration.y},
            {"z", mSettings.Acceleration.z}
        };

        properties["start_size"] = {
            {"x", mSettings.StartSize.x},
            {"y", mSettings.StartSize.y}
        };

        properties["end_size"] = {
            {"x", mSettings.EndSize.x},
            {"y", mSettings.EndSize.y}
        };

        properties["start_color"] = {
            {"x", mSettings.StartColor.x},
            {"y", mSettings.StartColor.y},
            {"z", mSettings.StartColor.z},
            {"w", mSettings.StartColor.w}
        };

        properties["end_color"] = {
            {"x", mSettings.EndColor.x},
            {"y", mSettings.EndColor.y},
            {"z", mSettings.EndColor.z},
            {"w", mSettings.EndColor.w}
        };

        if (!mMaterialPath.empty()) {
            properties["material"] = {
                {"path", mMaterialPath}
            };
        }

        return true;
    }

    void CpuParticleComponent::Start() {
        mParticles.reserve(mSettings.MaxParticles);
        if (mSettings.Shape == ParticleShape::Quad && !mMesh) {
            mMesh = Mesh::CreateQuad();
        } else if (mSettings.Shape == ParticleShape::Line && !mMesh) {
            mMesh = Mesh::CreateCube(glm::vec3(0.02f, 1.0f, 0.02f));
        }

        mPlaying = mSettings.Enabled;
    }

    void CpuParticleComponent::Update(float deltaTime) {
        if (!mPlaying || mSettings.MaxParticles == 0 || !GetOwner()) {
            return;
        }

        deltaTime = std::clamp(deltaTime, 0.0f, 0.1f);
        for (size_t index = 0; index < mParticles.size();) {
            Particle& particle = mParticles[index];
            particle.Age += deltaTime;
            if (particle.Age >= particle.Lifetime) {
                mParticles[index] = mParticles.back();
                mParticles.pop_back();
                continue;
            }

            particle.Velocity += mSettings.Acceleration * deltaTime;
            particle.Position += particle.Velocity * deltaTime;
            ++index;
        }

        if (mSettings.EmissionRate > 0.0f) {
            mEmissionAccumulator += mSettings.EmissionRate * deltaTime;
            const uint32_t spawnCount = static_cast<uint32_t>(mEmissionAccumulator);
            mEmissionAccumulator -= static_cast<float>(spawnCount);
            for (uint32_t index = 0; index < spawnCount; ++index) {
                Emit();
            }
        }

        SubmitRenderCommand();
    }

    void CpuParticleComponent::OnEnable() {
        mPlaying = mSettings.Enabled;
    }

    void CpuParticleComponent::OnDisable() {
        mPlaying = false;
        mParticles.clear();
        mEmissionAccumulator = 0.0f;
    }

    void CpuParticleComponent::SetSettings(const ParticleEmitterSettings& settings) {
        mSettings = settings;
        mPlaying  = settings.Enabled;
        mParticles.clear();
        mParticles.reserve(mSettings.MaxParticles);
    }

    const ParticleEmitterSettings& CpuParticleComponent::GetSettings() const {
        return mSettings;
    }

    void CpuParticleComponent::SetMaterial(const Ref<Material>& material) {
        mMaterial = material;
        if (mMaterial && mMaterial->GetShader()) {
            mMaterial->GetShader()->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        }
    }

    Ref<Material> CpuParticleComponent::GetMaterial() const {
        return mMaterial;
    }

    void CpuParticleComponent::Play() {
        mPlaying          = true;
        mSettings.Enabled = true;
    }

    void CpuParticleComponent::Stop() {
        mPlaying          = false;
        mSettings.Enabled = false;
        mParticles.clear();
    }

    void CpuParticleComponent::Restart() {
        mParticles.clear();
        mEmissionAccumulator = 0.0f;
        Play();
    }

    bool CpuParticleComponent::IsPlaying() const {
        return mPlaying;
    }

    void CpuParticleComponent::Emit() {
        if (mParticles.size() >= mSettings.MaxParticles) {
            return;
        }

        Particle particle;
        Random& random    = Random::GetInstance();
        particle.Position = mSettings.Position
                          + glm::vec3(random.NextSigned() * mSettings.PositionVariance.x,
                                      random.NextSigned() * mSettings.PositionVariance.y,
                                      random.NextSigned() * mSettings.PositionVariance.z);

        particle.Velocity = mSettings.Velocity
                          + glm::vec3(random.NextSigned() * mSettings.VelocityVariance.x,
                                      random.NextSigned() * mSettings.VelocityVariance.y,
                                      random.NextSigned() * mSettings.VelocityVariance.z);

        particle.Lifetime  = std::max(0.01f, mSettings.Lifetime + random.NextSigned() * mSettings.LifetimeVariance);
        particle.SizeScale = std::max(0.01f, 1.0f + random.NextSigned() * mSettings.SizeVariance);

        mParticles.push_back(particle);
    }

    void CpuParticleComponent::SubmitRenderCommand() const {
        if (mParticles.empty() || !mMaterial) {
            return;
        }

        const glm::mat4 world = GetOwner()->GetWorldTransform();

        if (mSettings.Shape == ParticleShape::Mesh) {
            if (!mMesh) {
                return;
            }

            for (const Particle& particle : mParticles) {
                const float age      = std::clamp(particle.Age / particle.Lifetime, 0.0f, 1.0f);
                const glm::vec2 size = glm::mix(mSettings.StartSize, mSettings.EndSize, age) * particle.SizeScale;

                RenderCommand command;
                command.Mesh     = mMesh.get();
                command.Material = mMaterial.get();
                command.Model    = world * glm::translate(glm::mat4(1.0f), particle.Position);
                command.Model    = glm::scale(command.Model, glm::vec3(size.x, size.y, size.x));
                Engine::GetInstance().GetCommandQueue().Submit(command);
            }

            return;
        }

        if (!mMaterial->IsUnlit()) {
            if (!mMesh) {
                return;
            }

            for (const Particle& particle : mParticles) {
                const float age      = std::clamp(particle.Age / particle.Lifetime, 0.0f, 1.0f);
                const glm::vec2 size = glm::mix(mSettings.StartSize, mSettings.EndSize, age) * particle.SizeScale;

                RenderCommand command;
                command.Mesh     = mMesh.get();
                command.Material = mMaterial.get();
                command.Model    = world * glm::translate(glm::mat4(1.0f), particle.Position);
               
                if (mSettings.Shape == ParticleShape::Line) {
                    command.Model = glm::scale(command.Model, glm::vec3(1.0f, mSettings.LineLength, 1.0f));
                } else {
                    command.Model = glm::scale(command.Model, glm::vec3(size.x, size.y, 1.0f));
                }

                Engine::GetInstance().GetCommandQueue().Submit(command);
            }
            return;
        }

        std::vector<float> vertices;
        vertices.reserve(mParticles.size() * (mSettings.Shape == ParticleShape::Line ? 2 : 6) * kFloatsPerVertex);

        auto append_vertex = [&vertices](const glm::vec3& position, const glm::vec4& color, const glm::vec2& uv) {
            vertices.insert(vertices.end(), {position.x, position.y, position.z, color.r, color.g, color.b, color.a, uv.x, uv.y});
        };

        for (const Particle& particle : mParticles) {
            const float age        = std::clamp(particle.Age / particle.Lifetime, 0.0f, 1.0f);
            const glm::vec4 color  = glm::mix(mSettings.StartColor, mSettings.EndColor, age);
            const glm::vec3 center = glm::vec3(world * glm::vec4(particle.Position, 1.0f));

            if (mSettings.Shape == ParticleShape::Line) {
                const glm::vec3 direction = glm::normalize(particle.Velocity + glm::vec3(0.0f, -0.001f, 0.0f));
                const glm::vec3 end       = center - direction * mSettings.LineLength;
                append_vertex(center, color, {0.0f, 0.0f});
                append_vertex(end, color, {1.0f, 0.0f});
                continue;
            }

            const glm::vec2 size  = glm::mix(mSettings.StartSize, mSettings.EndSize, age) * particle.SizeScale;
            const glm::vec3 right = glm::vec3(world[0]) * (size.x * 0.5f);
            const glm::vec3 up    = glm::vec3(world[1]) * (size.y * 0.5f);
            const glm::vec3 p0    = center - right - up;
            const glm::vec3 p1    = center + right - up;
            const glm::vec3 p2    = center + right + up;
            const glm::vec3 p3    = center - right + up;

            append_vertex(p0, color, {0.0f, 0.0f});
            append_vertex(p1, color, {1.0f, 0.0f});
            append_vertex(p2, color, {1.0f, 1.0f});
            append_vertex(p0, color, {0.0f, 0.0f});
            append_vertex(p2, color, {1.0f, 1.0f});
            append_vertex(p3, color, {0.0f, 1.0f});
        }

        UnlitCommand command;
        command.Material  = mMaterial.get();
        command.Vertices  = std::move(vertices);
        command.Primitive = mSettings.Shape == ParticleShape::Line ? PrimitiveType::Lines : PrimitiveType::Triangles;
        Engine::GetInstance().GetCommandQueue().Submit(command);
    }


} // namespace golias
