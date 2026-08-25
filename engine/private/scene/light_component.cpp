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

    bool LightComponent::LoadProperties(const Json& properties) {
        if (properties.contains("color")) {
            const auto& colorObj = properties["color"];
            mColor.x             = colorObj.value("r", 1.0f);
            mColor.y             = colorObj.value("g", 1.0f);
            mColor.z             = colorObj.value("b", 1.0f);
        }

        mIntensity = properties.value("intensity", 1.0f);
        mRange     = properties.value("range", 10.0f);
        mSpotAngle = properties.value("angle", 45.0f);

        String typeStr = properties.value("light_type", "directional");
        if (typeStr == "directional") {
            mType = LightType::Directional;
        } else if (typeStr == "point") {
            mType = LightType::Point;
        } else if (typeStr == "spot") {
            mType = LightType::Spot;
        } else {
            GOLIAS_LOG_WARN("LightComponent: Unknown light type '%s'. Defaulting to Directional.", typeStr.data());
            mType = LightType::Directional;
        }

        mIsShadowCaster = properties.value("casts_shadows", false);

        return true;
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
