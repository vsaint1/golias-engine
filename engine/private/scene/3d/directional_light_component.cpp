#include "scene/3d/directional_light_component.h"

#include "core/engine.h"

namespace golias {

    void DirectionalLightComponent::Start() {
    }

    void DirectionalLightComponent::Update(float deltaTime) {

        DirectionalLightCommand command;
        command.direction   = direction;
        command.color       = color;
        command.intensity   = intensity;
        command.castShadows = castShadows;
        Engine::GetInstance().GetSceneRenderer().Submit(command);
    }

    void DirectionalLightComponent::LoadProperties(const nlohmann::json& json) {
    }

    void DirectionalLightComponent::SetDirection(const glm::vec3& dir) {
        direction = dir;
    }

    glm::vec3 DirectionalLightComponent::GetDirection() const {
        return direction;
    }

    void DirectionalLightComponent::SetColor(const glm::vec3& value) {
        color = value;
    }

    glm::vec3 DirectionalLightComponent::GetColor() const {
        return color;
    }

    void DirectionalLightComponent::SetIntensity(float value) {
        intensity = value;
    }

    float DirectionalLightComponent::GetIntensity() const {
        return intensity;
    }

    void DirectionalLightComponent::SetCastShadows(bool enable) {
        castShadows = enable;
    }

    bool DirectionalLightComponent::GetCastShadows() const {
        return castShadows;
    }



} // namespace golias
