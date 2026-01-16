#pragma once

#include "scene/component.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace golias {

    class DirectionalLightComponent : public Component {
        COMPONENT(DirectionalLightComponent)
    public:
        DirectionalLightComponent()           = default;
        ~DirectionalLightComponent() override = default;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

        void SetDirection(const glm::vec3& dir) ;

        glm::vec3 GetDirection() const;

        void SetColor(const glm::vec3& value) ;
        glm::vec3 GetColor() const ;

        void SetIntensity(float value);
        float GetIntensity() const;

        void SetCastShadows(bool enable) ;
        bool GetCastShadows() const ;
        
    private:
        glm::vec3 direction = glm::vec3(0.5f, -1.0f, 0.3f);
        glm::vec3 color     = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity     = 1.0f;
        bool castShadows    = true;
    };
} // namespace golias
