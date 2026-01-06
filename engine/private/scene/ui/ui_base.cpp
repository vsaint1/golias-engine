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

} // namespace golias
