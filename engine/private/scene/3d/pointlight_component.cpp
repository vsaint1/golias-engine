#include "scene/3d/pointlight_component.h"

#include "core/engine.h"

namespace golias {

    void PointLightComponent::Start() {
    }

    void PointLightComponent::Update(float deltaTime) {
        PointLightCommand command;
        command.position    = position;
        command.color       = color;
        command.intensity   = intensity;
        command.range       = range;
        command.constant    = constant;
        command.linear      = linear;
        command.quadratic   = quadratic;
        command.castShadows = castShadows;

        Engine::GetInstance().GetSceneRenderer().Submit(command);
    }

    void PointLightComponent::LoadProperties(const nlohmann::json& json) {
    }

    void PointLightComponent::SetPosition(const glm::vec3& pos) {
        position = pos;
    }

    glm::vec3 PointLightComponent::GetPosition() const {
        return position;
    }

    void PointLightComponent::SetColor(const glm::vec3& col) {
        color = col;
    }

    glm::vec3 PointLightComponent::GetColor() const {
        return color;
    }

    void PointLightComponent::SetIntensity(float value) {
        intensity = value;
    }

    float PointLightComponent::GetIntensity() const {
        return intensity;
    }

    void PointLightComponent::SetRange(float value) {
        range = value;
    }

    float PointLightComponent::GetRange() const {
        return range;
    }

    void PointLightComponent::SetConstant(float constantTerm) {
        constant = constantTerm;
    }

    float PointLightComponent::GetConstant() const {
        return constant;
    }

    void PointLightComponent::SetLinear(float linearTerm) {
        linear = linearTerm;
    }

    float PointLightComponent::GetLinear() const {
        return linear;
    }

    void PointLightComponent::SetQuadratic(float quadraticTerm) {
        quadratic = quadraticTerm;
    }

    float PointLightComponent::GetQuadratic() const {
        return quadratic;
    }

    void PointLightComponent::SetCastShadows(bool cast) {
        castShadows = cast;
    }

    bool PointLightComponent::IsCastingShadows() const {
        return castShadows;
    }

} // namespace golias
