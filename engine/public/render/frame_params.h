#pragma once
#include "graphics/gpu_types.h"
#include "graphics/graphics_device.h"

namespace golias {

    struct CameraCommand;
    struct LightCommand;
    struct RenderCommand;
    struct SkySettings;

    /// @brief  Pure functions that assemble the GPU-layout structs from render commands. No state, no
    ///         ownership - passes call these to fill in the value they then hand to a ShaderParameter<T>.
    GpuLighting BuildGpuLighting(const std::vector<LightCommand>& lights,
                                 const glm::vec3& ambientColor = glm::vec3(0.08f));

    GpuFrame BuildGpuFrame(const CameraCommand& cameraCommand,
                           const std::array<glm::mat4, kMaxShadowCascades>& shadowMatrices,
                           const std::array<float, kMaxShadowCascades>& shadowSplits);

    GpuObject BuildGpuObject(const glm::mat4& model,
                             int instanceCount,
                             int isSkinned,
                             const glm::vec4& spritePivotSize = glm::vec4(0.0f),
                             const glm::vec4& spriteUvBounds  = glm::vec4(0.0f));

    // TODO: Add support for additional material properties such as roughness, metallic, and normal maps.
    GpuMaterial BuildGpuMaterial(const glm::vec4& baseColor);

    GpuSkybox BuildGpuSkybox(const CameraCommand& cameraCommand, const SkySettings& settings);


    class JointParameterCache {
    public:
        /// @brief  Uploads (if the version changed) and binds the command's joint matrices at JointsBinding.
        void Update(GraphicsDevice& device, const RenderCommand& command);

    private:
        std::map<const void*, std::pair<Ref<Buffer>, uint64_t>> mBuffers = {};
    };


} // namespace golias
