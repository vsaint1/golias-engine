#include "scene/ui/text_component.h"

namespace golias {
    const std::string& TextWidgetComponent::GetText() const {
        return text;
    }

    void TextWidgetComponent::SetText(const std::string& txt) {
        text = txt;
    }

    void TextWidgetComponent::Draw(CanvasComponent* pCanvas) {
       
    }
}