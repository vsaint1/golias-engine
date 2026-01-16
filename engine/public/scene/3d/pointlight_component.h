#pragma once
#include "scene/component.h"

#include <glm/vec3.hpp>

namespace golias {

    class PointLightComponent : public Component {
        COMPONENT(PointLightComponent)
    public:
        PointLightComponent()           = default;
        ~PointLightComponent() override = default;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

        void SetPosition(const glm::vec3& pos);
        glm::vec3 GetPosition() const;

        void SetColor(const glm::vec3& col);
        glm::vec3 GetColor() const;

        void SetIntensity(float value);
        float GetIntensity() const;

        void SetRange(float value);
        float GetRange() const;

        void SetConstant(float constantTerm);
        float GetConstant() const;

        void SetLinear(float linearTerm);
        float GetLinear() const;

        void SetQuadratic(float quadraticTerm);
        float GetQuadratic() const;

        void SetCastShadows(bool cast);
        bool IsCastingShadows() const;

    private:
        glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
        glm::vec3 color    = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity    = 1.0f;
        float range        = 10.0f; // Maximum distance of light influence
        float constant     = 1.0f; // Constant attenuation term
        float linear       = 0.09f; // Linear attenuation term
        float quadratic    = 0.032f; // Quadratic attenuation term
        bool castShadows   = false;
    };
} // namespace  golias
