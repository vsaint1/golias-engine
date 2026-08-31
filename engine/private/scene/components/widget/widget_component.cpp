#include "scene/components/widget/widget_component.h"

#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    void WidgetComponent::Update(float deltaTime) {
    }

    void WidgetComponent::Render(CanvasComponent* canvas) {
    }

    bool WidgetComponent::HitTest(const glm::vec2& point) {

        return false;
    }

    glm::vec2 WidgetComponent::GetDesiredSize() const {
        if (RectTransformComponent* rt = GetOwner()->GetComponent<RectTransformComponent>()) {
            return rt->GetSize();
        }

        return glm::vec2(0.0f);
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
