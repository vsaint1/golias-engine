#include "scene/3d/spotlight_component.h"

#include "core/engine.h"

namespace golias {

    float SpotlightComponent::PrecomputeConeAngleCosines(float angleInDegrees) {
        return glm::cos(glm::radians(angleInDegrees));
    }


    void SpotlightComponent::SetPosition(const glm::vec3& pos) {
        position = pos;
    }

    glm::vec3 SpotlightComponent::GetPosition() const {
        return position;
    }

    void SpotlightComponent::SetDirection(const glm::vec3& dir) {
        direction = dir;
    }

    glm::vec3 SpotlightComponent::GetDirection() const {
        return direction;
    }

    void SpotlightComponent::SetColor(const glm::vec3& col) {
        color = col;
    }

    glm::vec3 SpotlightComponent::GetColor() const {
        return color;
    }

    void SpotlightComponent::SetIntensity(float value) {
        intensity = value;
    }

    float SpotlightComponent::GetIntensity() const {
        return intensity;
    }

    void SpotlightComponent::SetRange(float value) {
        range = value;
    }


    float SpotlightComponent::GetRange() const {
        return range;
    }

    void SpotlightComponent::SetInnerConeAngle(float angle) {
        innerConeAngle = angle;
    }

    float SpotlightComponent::GetInnerConeAngle() const {
        return innerConeAngle;
    }

    void SpotlightComponent::SetOuterConeAngle(float angle) {
        outerConeAngle = angle;
    }

    float SpotlightComponent::GetOuterConeAngle() const {
        return outerConeAngle;
    }

    void SpotlightComponent::SetConstant(float constantTerm) {
        constant = constantTerm;
    }

    float SpotlightComponent::GetConstant() const {
        return constant;
    }

    void SpotlightComponent::SetLinear(float linearTerm) {
        linear = linearTerm;
    }

    float SpotlightComponent::GetLinear() const {
        return linear;
    }

    void SpotlightComponent::SetQuadratic(float quadraticTerm) {
        quadratic = quadraticTerm;
    }

    float SpotlightComponent::GetQuadratic() const {
        return quadratic;
    }

    void SpotlightComponent::SetCastShadows(bool cast) {
        castShadows = cast;
    }

    bool SpotlightComponent::IsCastingShadows() const {
        return castShadows;
    }

    void SpotlightComponent::Start() {
    }

    void SpotlightComponent::Update(float deltaTime) {

        SpotLightCommand spotLight;
        spotLight.position          = position;
        spotLight.direction         = direction;
        spotLight.color             = color;
        spotLight.intensity         = intensity;
        spotLight.innerConeAngle    = innerConeAngle;
        spotLight.outerConeAngle    = outerConeAngle;
        spotLight.range             = range;
        spotLight.constant          = constant;
        spotLight.innerConeAngleCos = PrecomputeConeAngleCosines(innerConeAngle);
        spotLight.outerConeAngleCos = PrecomputeConeAngleCosines(outerConeAngle);
        spotLight.linear            = linear;
        spotLight.quadratic         = quadratic;
        spotLight.castShadows       = castShadows;
        Engine::GetInstance().GetSceneRenderer().Submit(spotLight);
    }

    void SpotlightComponent::LoadProperties(const nlohmann::json& json) {
    }
} // namespace golias
