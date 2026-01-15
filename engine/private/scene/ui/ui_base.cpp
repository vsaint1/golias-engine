#include "scene/ui/ui_base.h"

namespace golias {

    glm::vec2 WidgetComponent::GetPivot() const {
        return pivot;
    }

    void WidgetComponent::SetPivot(const glm::vec2& value) {
        pivot = value;
    }

    void WidgetComponent::Draw(CanvasComponent* pCanvas) {
    }

    bool WidgetComponent::HitTest(const glm::vec2& point) const {
        return false;
    }

    void WidgetComponent::OnPointerEnter() {
    }

    void WidgetComponent::OnPointerExit() {
    }

    void WidgetComponent::OnPointerDown() {
    }

    void WidgetComponent::OnPointerUp() {
    }

    void WidgetComponent::OnClick() {
    }

} // namespace golias
