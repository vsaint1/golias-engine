#pragma once
#include "scene/ui/ui_base.h"

namespace golias {

    class TextWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(TextWidgetComponent, WidgetComponent)

    public:
        const std::string& GetText() const;

        void SetText(const std::string& txt);

        void Draw(CanvasComponent* pCanvas) override;

    protected:
        std::string text;
    };
} // namespace golias
