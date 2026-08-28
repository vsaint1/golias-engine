#include "render/csm.h"

namespace golias {

    void CascadedShadowMapDesc::Clamp() {
        CascadeCount        = std::clamp(CascadeCount, 1u, kMaxCascades);
        ShadowMapResolution = std::max(ShadowMapResolution, 1u);
        SplitLambda         = std::clamp(SplitLambda, 0.0f, 1.0f);
        MaxDistance         = std::max(MaxDistance, 0.1f);
        Bias                = std::max(Bias, 0.0f);
        NormalBias          = std::max(NormalBias, 0.0f);
    }


    static glm::vec3 unproject(const glm::mat4& inverseMatrix, const glm::vec3& ndc) {
        const glm::vec4 world = inverseMatrix * glm::vec4(ndc, 1.0f);
        return glm::vec3(world) / world.w;
    }

    void CascadedShadowMap::RebuildSplits(float nearPlane, float farPlane) {
        mSplits.fill(farPlane);
        for (uint32_t i = 0; i < mSettings.CascadeCount; ++i) {
            const float fraction    = static_cast<float>(i + 1) / static_cast<float>(mSettings.CascadeCount);
            const float logarithmic = nearPlane * std::pow(farPlane / nearPlane, fraction);
            const float uniform     = nearPlane + (farPlane - nearPlane) * fraction;
            mSplits[i]              = glm::mix(uniform, logarithmic, mSettings.SplitLambda);
        }
    }


    CascadedShadowMap::CascadedShadowMap(const CascadedShadowMapDesc& settings) {
        Prepare(settings);
    }

    void CascadedShadowMap::Prepare(const CascadedShadowMapDesc& settings) {
        mSettings = settings;
        mSettings.Clamp();
    }

    void CascadedShadowMap::Build(const glm::mat4& cameraView,
                                  const glm::mat4& cameraProjection,
                                  const glm::vec3& lightDirection,
                                  float cameraNear,
                                  float cameraFar) {

        const float nearPlane = std::max(cameraNear, 0.001f);
        const float farPlane  = std::max(cameraFar, nearPlane);

        // TODO: Make this rebuild only if changes (minor changes)
        RebuildSplits(nearPlane, farPlane);

        const glm::mat4 inverseCamera = glm::inverse(cameraProjection * cameraView);

        const glm::vec3 nearCorners[4] = {unproject(inverseCamera, {-1.0f, -1.0f, 0.0f}),
                                          unproject(inverseCamera, {1.0f, -1.0f, 0.0f}),
                                          unproject(inverseCamera, {-1.0f, 1.0f, 0.0f}),
                                          unproject(inverseCamera, {1.0f, 1.0f, 0.0f})};

        const glm::vec3 farCorners[4] = {unproject(inverseCamera, {-1.0f, -1.0f, 1.0f}),
                                         unproject(inverseCamera, {1.0f, -1.0f, 1.0f}),
                                         unproject(inverseCamera, {-1.0f, 1.0f, 1.0f}),
                                         unproject(inverseCamera, {1.0f, 1.0f, 1.0f})};

        glm::vec3 direction = lightDirection;
        if (glm::length2(direction) < std::numeric_limits<float>::epsilon()) {
            direction = glm::vec3(0.0f, -1.0f, 0.0f);
        } else {
            direction = glm::normalize(direction);
        }

        for (uint32_t cascade = 0; cascade < mSettings.CascadeCount; ++cascade) {
            const float previousSplit = cascade == 0 ? nearPlane : std::min(mSplits[cascade - 1], farPlane);
            const float currentSplit  = std::min(mSplits[cascade], farPlane);
            const float previousRatio = glm::clamp((previousSplit - nearPlane) / (farPlane - nearPlane), 0.0f, 1.0f);
            const float currentRatio  = glm::clamp((currentSplit - nearPlane) / (farPlane - nearPlane), 0.0f, 1.0f);

            std::array<glm::vec3, 8> corners;
            glm::vec3 center(0.0f);
            for (size_t i = 0; i < 4; ++i) {
                corners[i]     = glm::mix(nearCorners[i], farCorners[i], previousRatio);
                corners[i + 4] = glm::mix(nearCorners[i], farCorners[i], currentRatio);
                center += corners[i] + corners[i + 4];
            }
            center /= 8.0f;

            float radius = 0.0f;
            for (const glm::vec3& corner : corners) {
                radius = std::max(radius, glm::length(corner - center));
            }
            radius = std::max(radius, 0.001f);

            const glm::vec3 up = std::abs(glm::dot(direction, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f)
                                                                                                    : glm::vec3(0.0f, 1.0f, 0.0f);

            // cascade center
            const glm::mat4 lightBasis       = glm::lookAtLH(-direction, glm::vec3(0.0f), up);
            const float texelSize            = (2.0f * radius) / static_cast<float>(mSettings.ShadowMapResolution);
            const glm::vec3 lightSpaceCenter = glm::mat3(lightBasis) * center;
            const glm::vec3 snappedCenter(std::floor(lightSpaceCenter.x / texelSize + 0.5f) * texelSize,
                                          std::floor(lightSpaceCenter.y / texelSize + 0.5f) * texelSize,
                                          lightSpaceCenter.z);

            center += glm::inverse(glm::mat3(lightBasis)) * (snappedCenter - lightSpaceCenter);

            const glm::mat4 lightView = glm::lookAtLH(center - direction * radius, center, up);

            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::lowest();
            for (const glm::vec3& corner : corners) {
                const float z = (lightView * glm::vec4(corner, 1.0f)).z;
                minZ          = std::min(minZ, z);
                maxZ          = std::max(maxZ, z);
            }

            const float depthMargin = radius;

            mCascades[cascade].ViewProjection =
                glm::orthoLH_NO(-radius, radius, -radius, radius, minZ - depthMargin, maxZ + depthMargin) * lightView;

            mCascades[cascade].SplitDistance = currentSplit;
        }
    }

    const CascadedShadowMapDesc& CascadedShadowMap::GetSettings() const {
        return mSettings;
    }

    const float CascadedShadowMap::GetSplit(uint32_t cascade) const {
        return mSplits[cascade];
    }

    const std::array<float, CascadedShadowMapDesc::kMaxCascades>& CascadedShadowMap::GetSplits() const {
        return mSplits;
    }

    const Cascade& CascadedShadowMap::GetCascade(uint32_t cascade) const {
        return mCascades[cascade];
    }


    const std::array<Cascade, CascadedShadowMapDesc::kMaxCascades>& CascadedShadowMap::GetCascades() const {
        return mCascades;
    }

} // namespace golias
