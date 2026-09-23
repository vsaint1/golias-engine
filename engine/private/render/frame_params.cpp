#include "render/frame_params.h"

#include "render/command_queue.h"
#include "render/passes/skybox_pass.h"

namespace golias {

    GpuLighting BuildGpuLighting(const std::vector<LightCommand>& lights, const glm::vec3& ambientColor) {
        GpuLighting lighting = {
            .Count        = static_cast<int>(std::min(lights.size(), kMaxLights)),
            .AmbientColor = glm::vec4(ambientColor, 1.0f),
        };

        for (size_t i = 0; i < static_cast<size_t>(lighting.Count); ++i) {
            const LightCommand& source = lights[i];
            GpuLight& destination      = lighting.Lights[i];

            destination.Position = glm::vec4(source.Position, 1.0f);
            const glm::vec3 direction =
                glm::length2(source.Direction) > 1e-8f ? glm::normalize(source.Direction) : glm::vec3(0.0f, -1.0f, 0.0f);
            destination.DirectionInvRange = glm::vec4(direction, 1.0f / std::max(source.Range, 1e-6f));
            destination.ColorIntensity    = glm::vec4(source.Color, source.Intensity);
            destination.SpotCutoff        = std::cos(glm::radians(source.SpotAngle));
            destination.Type              = source.Type;
            destination.IsShadowCaster    = source.IsShadowCaster ? 1 : 0;
        }

        return lighting;
    }

    GpuFrame BuildGpuFrame(const CameraCommand& cameraCommand,
                           const std::array<glm::mat4, kMaxShadowCascades>& shadowMatrices,
                           const std::array<float, kMaxShadowCascades>& shadowSplits) {
        GpuFrame frame       = {};
        frame.View           = cameraCommand.View;
        frame.Projection     = cameraCommand.Projection;
        frame.Ortho          = cameraCommand.Ortho;
        frame.CameraPosition = glm::vec4(cameraCommand.CameraPosition, 1.0f);
        for (uint32_t cascade = 0; cascade < kMaxShadowCascades; ++cascade) {
            frame.ShadowMatrices[cascade] = shadowMatrices[cascade];
            frame.ShadowSplits[cascade]   = shadowSplits[cascade];
        }

        return frame;
    }

    GpuObject BuildGpuObject(
        const glm::mat4& model, int instanceCount, int isSkinned, const glm::vec4& spritePivotSize, const glm::vec4& spriteUvBounds) {
        GpuObject object       = {};
        object.Model           = model;
        object.Flags           = glm::ivec4(instanceCount, isSkinned, 0, 0);
        object.SpritePivotSize = spritePivotSize;
        object.SpriteUvBounds  = spriteUvBounds;
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        object.NormalMatrix[0] = glm::vec4(normalMatrix[0], 0.0f);
        object.NormalMatrix[1] = glm::vec4(normalMatrix[1], 0.0f);
        object.NormalMatrix[2] = glm::vec4(normalMatrix[2], 0.0f);

        return object;
    }

    GpuMaterial BuildGpuMaterial(const glm::vec4& baseColor) {
        return GpuMaterial{.BaseColor = baseColor};
    }

    GpuSkybox BuildGpuSkybox(const CameraCommand& cameraCommand, const SkySettings& settings) {
        GpuSkybox sky  = {};
        sky.Projection = cameraCommand.Projection;

        glm::mat4 viewRotation = cameraCommand.View;
        viewRotation[3]        = glm::vec4(0.0f);

        sky.ViewRotation        = viewRotation;
        sky.InverseProjection   = glm::inverse(cameraCommand.Projection);
        sky.InverseViewRotation = glm::inverse(viewRotation);
        sky.ViewportSize =
            glm::vec4(static_cast<float>(cameraCommand.Viewport.Width), static_cast<float>(cameraCommand.Viewport.Height), 0.0f, 0.0f);

        sky.SkyTint      = glm::vec4(settings.SkyTint, 1.0f);
        sky.GroundColor  = glm::vec4(settings.GroundColor, 1.0f);
        sky.SunDirection = glm::vec4(glm::normalize(settings.SunDirection), 0.0f);

        sky.Parameters = glm::vec4(settings.AtmosphereThickness, settings.Exposure, settings.TextureAngularSize, settings.BrightnessFactor);

        sky.SunSettings = glm::vec4(settings.SunBrightness, settings.TextureAngularSize, settings.StarIntensity, 0.0f);
        
        return sky;
    }


    void JointParameterCache::Update(GraphicsDevice& device, const RenderCommand& command) {
        if (!command.JointMatrices || command.JointCount == 0) {
            return;
        }

        auto& [buffer, inFlightVersion] = mBuffers[command.JointKey];
        if (!buffer) {
            // clang-format off
            BufferDesc desc = {
                .Target = BufferTarget::Uniform,
                .Usage  = BufferUsage::Dynamic,
                .Size   = kMaxJoints * sizeof(glm::mat4)
            };
            // clang-format on

            buffer          = device.CreateBuffer(desc);
            inFlightVersion = static_cast<uint64_t>(-1);
        }

        const uint32_t safeJointCount = std::min(command.JointCount, kMaxJoints);
        if (inFlightVersion != command.JointVersion) {
            inFlightVersion = command.JointVersion;
            buffer->Update(command.JointMatrices, safeJointCount * sizeof(glm::mat4));
        }

        buffer->Bind(GpuLayout::JointsBinding);
    }

} // namespace golias
