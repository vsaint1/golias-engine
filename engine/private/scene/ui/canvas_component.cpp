#include "scene/ui/canvas_component.h"

#include "scene/game_object.h"

namespace golias {
    void CanvasComponent::Update(float deltaTime) {
        const auto& children = GetOwner()->GetChildren();

        for (const auto& child : children) {
            if (auto pComponent = child->GetComponent<WidgetComponent>()) {
                Draw(pComponent);
            }
        }
    }

    void CanvasComponent::Draw(WidgetComponent* pWidget) {

        if (!pWidget) {
            return;
        }

        pWidget->Draw(this);

        const auto& children = GetOwner()->GetChildren();
        for (const auto& child : children) {
            if (auto pComponent = child->GetComponent<WidgetComponent>()) {
                Draw(pComponent);
            }
        }
    }
} // namespace golias
