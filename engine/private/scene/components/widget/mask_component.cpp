#include "scene/components/widget/mask_component.h"

#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    bool MaskComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        return true;
    }

    void MaskComponent::Render(CanvasComponent* canvas) {
        UNUSED_PARAMETER(canvas);
    }

    bool MaskComponent::GetMaskRect(glm::vec2& lowerLeft, glm::vec2& size) const {
        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return false;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        size                      = rectTransform->GetSize();
        lowerLeft                 = screenPos - rectTransform->GetPivot() * size;

        return true;
    }

} // namespace golias
