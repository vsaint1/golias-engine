#include "scene/ui/text_component.h"

#include "core/engine.h"
#include "scene/ui/canvas_component.h"

namespace golias {
    const std::string& TextWidgetComponent::GetText() const {
        return text;
    }

    void TextWidgetComponent::SetText(const std::string& txt) {
        text = txt;
    }

    glm::vec4 TextWidgetComponent::GetTextColor() const {
        return textColor;
    }

    void TextWidgetComponent::SetTextColor(const glm::vec4& color) {
        textColor = color;
    }

    void TextWidgetComponent::SetFont(const std::shared_ptr<Font>& pFont) {
        font = pFont;
    }

    void TextWidgetComponent::SetFont(const std::string_view pFilePath, int size) {
        font = Engine::GetInstance().GetFontManager().GetFont(pFilePath, size);
    }

    const std::shared_ptr<Font>& TextWidgetComponent::GetFont() const {
        return font;
    }

    void TextWidgetComponent::LoadProperties(const nlohmann::json& json) {

        text = json.value("text", text);

        if (json.contains("color")) {
            const auto& col = json["color"];
            glm::vec4 value = {col.value("r", 1.0f), col.value("g", 1.0f), col.value("b", 1.0f), col.value("a", 1.0f)};
            SetTextColor(value);
        }

        if (json.contains("font")) {
            const auto& fontObj        = json["font"];
            const std::string fontPath = fontObj.value("path", "");
            const int fontSize         = fontObj.value("size", 16);

            SetFont(fontPath, fontSize);
        }
    }

    void TextWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (text.empty() || !font) {
            return;
        }


        
        int w = font->GetTexture()->GetWidth();
        int h = font->GetTexture()->GetHeight();
        
        auto pos = GetPivotPos();
        float cursorX = pos.x;
        float cursorY = pos.y;

        for (const char& c : text) {

            const Glyph& glyph = font->GetGlyph(c);

            float x1 = cursorX;
            float y1 = cursorY;
            float x2 = cursorX + static_cast<float>(glyph.width);
            float y2 = cursorY + static_cast<float>(glyph.height);

            float u1 = static_cast<float>(glyph.x0) / static_cast<float>(w);
            float v1 = static_cast<float>(glyph.y0) / static_cast<float>(h);

            float u2 = static_cast<float>(glyph.x1) / static_cast<float>(w);
            float v2 = static_cast<float>(glyph.y1) / static_cast<float>(h);

            cursorX += static_cast<float>(glyph.advance);

            pCanvas->DrawTexture2D(
                glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec2(u1, v1), glm::vec2(u2, v2), font->GetTexture().get(), textColor);
        }
    }

    glm::vec2 TextWidgetComponent::GetPivotPos() const {
        auto pos = GetOwner()->GetWorldPosition2D();

        glm::vec2 rect(0.0f);
        for (const char& c : text) {

            const Glyph& glyph = font->GetGlyph(c);
            rect.x += static_cast<float>(glyph.advance);
            rect.y = glm::max(rect.y, static_cast<float>(glyph.height));
        }

        pos.x -= SDL_roundf(rect.x * pivot.x);
        pos.y -= SDL_roundf(rect.y * pivot.y);

        return pos;
    }
} // namespace golias
