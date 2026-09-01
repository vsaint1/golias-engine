#pragma once
#include "stdafx.h"

namespace golias {

    struct CascadedShadowMapDesc {
        static constexpr uint32_t kMaxCascades = 4;

        bool Enabled                 = true;
        uint32_t CascadeCount        = 4;
        uint32_t ShadowMapResolution = 4092;
        float SplitLambda            = 0.5f;
        float MaxDistance            = 1000.0f;
        float Bias                   = 0.0005f;
        float NormalBias             = 0.001f;

        void Clamp();
    };

    struct Cascade {
        glm::mat4 ViewProjection = glm::mat4(1.0f);
        float SplitDistance      = 0.0f;
    };

    class CascadedShadowMap {
    public:
        CascadedShadowMap(const CascadedShadowMapDesc& settings = {});

        void Prepare(const CascadedShadowMapDesc& settings);

        void Build(const glm::mat4& cameraView,
                   const glm::mat4& cameraProjection,
                   const glm::vec3& lightDirection,
                   float cameraNear,
                   float cameraFar);

        const CascadedShadowMapDesc& GetSettings() const;

        const float GetSplit(uint32_t cascade) const;

        const std::array<float, CascadedShadowMapDesc::kMaxCascades>& GetSplits() const;

        const Cascade& GetCascade(uint32_t cascade) const;

        const std::array<Cascade, CascadedShadowMapDesc::kMaxCascades>& GetCascades() const;

    private:
        void RebuildSplits(float nearPlane, float farPlane);

    private:
        CascadedShadowMapDesc mSettings;
        std::array<float, CascadedShadowMapDesc::kMaxCascades> mSplits     = {};
        std::array<Cascade, CascadedShadowMapDesc::kMaxCascades> mCascades = {};
    };

} // namespace golias
