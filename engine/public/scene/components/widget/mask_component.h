#pragma once

#include "widget_component.h"

namespace golias {

    class CanvasComponent;

    class MaskComponent : public WidgetComponent {
        COMPONENT_DERIVED(MaskComponent, WidgetComponent)
    public:
        MaskComponent()  = default;
        ~MaskComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Render(CanvasComponent* canvas) override;

        bool GetMaskRect(glm::vec2& lowerLeft, glm::vec2& size) const override;
    };

} // namespace golias
