#include "scene/ui/toggle_component.h"

#include "core/engine.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void ToggleWidgetComponent::SetIsOn(bool on) {
        if (isOn != on) {
            isOn = on;
            if (OnValueChanged) {
                OnValueChanged(isOn);
            }
        }
    }

    bool ToggleWidgetComponent::IsOn() const {
        return isOn;
    }

    void ToggleWidgetComponent::SetInteractable(bool val) {
        interactable = val;
    }

    bool ToggleWidgetComponent::IsInteractable() const {
        return interactable;
    }

    void ToggleWidgetComponent::SetTransition(EToggleTransition trans) {
        transition = trans;
    }

    EToggleTransition ToggleWidgetComponent::GetTransition() const {
        return transition;
    }

    void ToggleWidgetComponent::SetBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }

    glm::vec4 ToggleWidgetComponent::GetBackgroundColor() const {
        return backgroundColor;
    }

    void ToggleWidgetComponent::SetCheckmarkColor(const glm::vec4& color) {
        checkmarkColor = color;
    }

    glm::vec4 ToggleWidgetComponent::GetCheckmarkColor() const {
        return checkmarkColor;
    }

    void ToggleWidgetComponent::SetHoverColor(const glm::vec4& color) {
        hoverColor = color;
    }

    glm::vec4 ToggleWidgetComponent::GetHoverColor() const {
        return hoverColor;
    }

    void ToggleWidgetComponent::SetCheckmarkPadding(float padding) {
        checkmarkPadding = padding;
    }

    float ToggleWidgetComponent::GetCheckmarkPadding() const {
        return checkmarkPadding;
    }

    void ToggleWidgetComponent::Start() {
        fadeAlpha = isOn ? 1.0f : 0.0f;
    }

    void ToggleWidgetComponent::Update(float deltaTime) {
        if (transition == EToggleTransition::FADE) {
            float targetAlpha = isOn ? 1.0f : 0.0f;
            float fadeSpeed = 8.0f;
            
            if (fadeAlpha < targetAlpha) {
                fadeAlpha = std::min(fadeAlpha + fadeSpeed * deltaTime, targetAlpha);
            } else if (fadeAlpha > targetAlpha) {
                fadeAlpha = std::max(fadeAlpha - fadeSpeed * deltaTime, targetAlpha);
            }
        } else {
            fadeAlpha = isOn ? 1.0f : 0.0f;
        }
    }

    void ToggleWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (!pCanvas) {
            return;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return;
        }

        auto pos = rt->GetScreenPosition();
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;

        // Draw background
        glm::vec4 bgColor = isHovered ? hoverColor : backgroundColor;
        if (!interactable) {
            bgColor *= 0.5f;
            bgColor.a = backgroundColor.a;
        }

        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            bgColor);

        // Draw checkmark if on (with potential fade)
        if (fadeAlpha > 0.0f) {
            float scaledPadding = checkmarkPadding * uiScale;
            glm::vec2 checkPos = pos + glm::vec2(scaledPadding);
            glm::vec2 checkSize = scaledSize - glm::vec2(scaledPadding * 2.0f);

            glm::vec4 checkColor = checkmarkColor;
            checkColor.a *= fadeAlpha;
            if (!interactable) {
                checkColor *= 0.7f;
                checkColor.a = checkmarkColor.a * fadeAlpha;
            }

            pCanvas->DrawTexture2D(
                glm::vec3(checkPos, 0.01f),
                glm::vec3(checkPos.x + checkSize.x, checkPos.y + checkSize.y, 0.01f),
                glm::vec2(0.0f),
                glm::vec2(1.0f),
                nullptr,
                checkColor);
        }
    }

    void ToggleWidgetComponent::LoadProperties(const nlohmann::json& json) {
        isOn = json.value("isOn", false);
        interactable = json.value("interactable", true);
        checkmarkPadding = json.value("checkmarkPadding", 4.0f);

        if (json.contains("backgroundColor")) {
            const auto& col = json["backgroundColor"];
            backgroundColor = {col.value("r", 0.4f), col.value("g", 0.4f), col.value("b", 0.4f), col.value("a", 1.0f)};
        }

        if (json.contains("checkmarkColor")) {
            const auto& col = json["checkmarkColor"];
            checkmarkColor = {col.value("r", 0.2f), col.value("g", 0.8f), col.value("b", 0.2f), col.value("a", 1.0f)};
        }

        if (json.contains("hoverColor")) {
            const auto& col = json["hoverColor"];
            hoverColor = {col.value("r", 0.5f), col.value("g", 0.5f), col.value("b", 0.5f), col.value("a", 1.0f)};
        }

        fadeAlpha = isOn ? 1.0f : 0.0f;
    }

    bool ToggleWidgetComponent::HitTest(const glm::vec2& point) const {
        if (!interactable) {
            return false;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return false;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto p1 = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        auto p2 = p1 + scaledSize;

        return (point.x >= p1.x) && (point.x <= p2.x) && (point.y >= p1.y) && (point.y <= p2.y);
    }

    void ToggleWidgetComponent::OnPointerEnter() {
        isHovered = true;
    }

    void ToggleWidgetComponent::OnPointerExit() {
        isHovered = false;
    }

    void ToggleWidgetComponent::OnClick() {
        if (interactable) {
            SetIsOn(!isOn);
        }
    }

} // namespace golias
