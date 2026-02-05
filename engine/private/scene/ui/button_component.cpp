#include "scene/ui/button_component.h"

#include "core/engine.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void ButtonWidgetComponent::SetColor(const glm::vec4& value) {
        normalColor = value;
        color       = value;
    }

    glm::vec4 ButtonWidgetComponent::GetColor() const {
        return color;
    }

    void ButtonWidgetComponent::Draw(CanvasComponent* pCanvas) {

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

        pCanvas->DrawTexture2D(glm::vec3(pos, 0.0f),
                             glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
                             glm::vec2(0.0f, 0.0f),
                             glm::vec2(1.0f, 1.0f),
                             nullptr,
                             color);
    }

    void ButtonWidgetComponent::Start() {
    }

    void ButtonWidgetComponent::Update(float deltaTime) {
    }

    bool ButtonWidgetComponent::HitTest(const glm::vec2& point) const {

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();

        if (!rt) {
            return false;
        }
        
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();

        auto p1 = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        auto p2 = p1 + scaledSize;

        bool inside = (point.x >= p1.x) && (point.x <= p2.x) && (point.y >= p1.y) && (point.y <= p2.y);

        // Debug: Log button bounds on first check
        static bool logged = false;
        if (!logged && GetOwner()) {
            spdlog::info("Button '{}' bounds: ({:.0f}, {:.0f}) to ({:.0f}, {:.0f})", 
                GetOwner()->GetName(), p1.x, p1.y, p2.x, p2.y);
            logged = true;
        }

        return inside;
    }

    void ButtonWidgetComponent::OnPointerEnter() {
        color = hoverColor;
    }

    void ButtonWidgetComponent::OnPointerExit() {
        color = normalColor;
    }

    void ButtonWidgetComponent::OnPointerDown() {
        color = pressedColor;
    }

    void ButtonWidgetComponent::OnPointerUp() {
        color = hoverColor;
    }

    void ButtonWidgetComponent::OnClick() {
        if (OnButtonClick) {
            OnButtonClick();
        }
    }
} // namespace golias
