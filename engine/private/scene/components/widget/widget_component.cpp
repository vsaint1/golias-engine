#include "scene/components/widget/widget_component.h"

#include "scene/components/widget/canvas_component.h"

namespace golias {

    void WidgetComponent::Update(float deltaTime) {
    }

    void WidgetComponent::Render(CanvasComponent* canvas) {
    }

    bool WidgetComponent::HitTest(const glm::vec2& point) {

        return false;
    }

    void WidgetComponent::OnPointerEnter() {
    }

    void WidgetComponent::OnPointerExit() {
    }

    void WidgetComponent::OnPointerUp() {
    }

    void WidgetComponent::OnPointerDown() {
    }

    void WidgetComponent::OnClick() {
    }


} // namespace golias
