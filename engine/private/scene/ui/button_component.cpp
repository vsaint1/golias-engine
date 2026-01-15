#include "scene/ui/button_component.h"

#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"

namespace golias {
    void ButtonWidgetComponent::SetRect(const glm::vec2& size) {
        rect = size;
    }

    glm::vec2 ButtonWidgetComponent::GetRect() const {
        return rect;
    }

    void ButtonWidgetComponent::SetColor(const glm::vec4& value) {
        color = value;
    }

    glm::vec4 ButtonWidgetComponent::GetColor() const {
        return color;
    }

    void ButtonWidgetComponent::Draw(CanvasComponent* pCanvas) {

        auto pos = GetOwner()->GetWorldPosition2D();
        pos.x -= rect.x * pivot.x;
        pos.y -= rect.y * pivot.y;

        pCanvas->DrawTexture2D(glm::vec3(pos.x, pos.y, 0.0f),
                               glm::vec3(pos.x + rect.x, pos.y + rect.y, 0.0f),
                               glm::vec2(0.0f, 1.0f), 
                               glm::vec2(1.0f, 0.0f), 
                               nullptr,
                               color);
    }

    void ButtonWidgetComponent::Start() {
    }

    void ButtonWidgetComponent::Update(float deltaTime) {
    }

    bool ButtonWidgetComponent::HitTest(const glm::vec2& point) const {
        auto ownerPosition = GetOwner()->GetWorldPosition2D();

        float x1 = ownerPosition.x - rect.x * pivot.x;
        float y1 = ownerPosition.y - rect.y * pivot.y;

        float x2 = x1 + rect.x;
        float y2 = y1 + rect.y;

        bool inside = (point.x >= x1 && point.x <= x2 && point.y >= y1 && point.y <= y2);

        return inside;
    }

    void ButtonWidgetComponent::OnPointerEnter() {
    }

    void ButtonWidgetComponent::OnPointerExit() {
    }

    void ButtonWidgetComponent::OnPointerDown() {
    }

    void ButtonWidgetComponent::OnPointerUp() {
    }

    void ButtonWidgetComponent::OnClick() {
    }
} // namespace golias
