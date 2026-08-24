#include "scene/components/light_component.h"

#include "core/engine.h"

namespace golias {

    const glm::vec3& LightComponent::GetColor() const {
        return mColor;
    }

    void LightComponent::SetColor(const glm::vec3& color) {
        mColor = color;
    }

    float LightComponent::GetIntensity() const {
        return mIntensity;
    }

    void LightComponent::SetIntensity(float intensity) {
        mIntensity = intensity;
    }

    float LightComponent::GetRange() const {
        return mRange;
    }

    void LightComponent::SetRange(float range) {
        mRange = std::max(0.001f, range);
    }

    float LightComponent::GetSpotAngle() const {
        return mSpotAngle;
    }

    void LightComponent::SetSpotAngle(float angle) {
        mSpotAngle = std::clamp(angle, 0.0f, 90.0f);
    }

    LightType LightComponent::GetType() const {
        return mType;
    }

    void LightComponent::SetType(LightType type) {
        mType = type;
    }


    bool LightComponent::IsShadowCastingEnabled() const {
        return mIsShadowCaster;
    }

    void LightComponent::SetShadowCastingEnabled(bool castsShadows) {
        mIsShadowCaster = castsShadows;
    }

    void LightComponent::Update(float deltaTime) {
        LightCommand light;
        light.Position       = GetOwner()->GetWorldPosition();
        light.Direction      = GetOwner()->GetForward();
        light.Color          = GetColor();
        light.Intensity      = GetIntensity();
        light.Range          = GetRange();
        light.SpotAngle      = GetSpotAngle();
        light.Type           = static_cast<int>(GetType());
        light.IsShadowCaster = IsShadowCastingEnabled();

        Engine::GetInstance().GetCommandQueue().Submit(light);
    }

} // namespace golias
