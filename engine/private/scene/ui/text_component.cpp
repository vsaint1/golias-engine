#include "scene/ui/text_component.h"

namespace golias {
    const std::string& TextComponent::GetText() const {
        return text;
    }

    void TextComponent::SetText(const std::string& txt) {
        text = txt;
    }

    void TextComponent::Draw(CanvasComponent* pCanvas) {
       
    }
}