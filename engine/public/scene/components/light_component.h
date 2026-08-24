#pragma once
#include "component.h"

namespace golias {

    enum class LightType { Directional, Point, Spot };

    class LightComponent : public Component {

        COMPONENT(LightComponent)
    public:
        LightComponent()           = default;
        ~LightComponent() override = default;

        void Update(float deltaTime) override;

        const glm::vec3& GetColor() const;
        void SetColor(const glm::vec3& color);

        float GetIntensity() const;
        void SetIntensity(float intensity);

        float GetRange() const;
        void SetRange(float range);

        float GetSpotAngle() const;
        void SetSpotAngle(float angle);

        LightType GetType() const;
        void SetType(LightType type);

        bool IsShadowCastingEnabled() const;
        void SetShadowCastingEnabled(bool castsShadows);

    private:
        glm::vec3 mColor     = glm::vec3(1.0f, 1.0f, 1.0f);
        float mIntensity     = 1.0f;
        float mRange         = 10.0f;
        float mSpotAngle     = 45.0f;
        LightType mType      = LightType::Directional;
        bool mIsShadowCaster = false;
    };

} // namespace golias
