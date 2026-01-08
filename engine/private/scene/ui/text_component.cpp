#include "scene/ui/text_component.h"

#include "core/engine.h"
#include "scene/ui/canvas_component.h"

namespace golias {

    static uint32_t DecodeUTF8(const char*& ptr) {
        uint32_t codepoint = 0;
        unsigned char c    = static_cast<unsigned char>(*ptr++);

        if (c < 0x80) {
            codepoint = c;
        } else if ((c & 0xE0) == 0xC0) {
            codepoint = (c & 0x1F) << 6;
            codepoint |= (static_cast<unsigned char>(*ptr++) & 0x3F);
        } else if ((c & 0xF0) == 0xE0) {
            codepoint = (c & 0x0F) << 12;
            codepoint |= (static_cast<unsigned char>(*ptr++) & 0x3F) << 6;
            codepoint |= (static_cast<unsigned char>(*ptr++) & 0x3F);
        } else if ((c & 0xF8) == 0xF0) {
            codepoint = (c & 0x07) << 18;
            codepoint |= (static_cast<unsigned char>(*ptr++) & 0x3F) << 12;
            codepoint |= (static_cast<unsigned char>(*ptr++) & 0x3F) << 6;
            codepoint |= (static_cast<unsigned char>(*ptr++) & 0x3F);
        }

        return codepoint;
    }

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

        if (!font) {
            spdlog::error("TextWidgetComponent::SetFont Failed to load font '{}' with size {}", pFilePath, size);
        }
    }

    const std::shared_ptr<Font>& TextWidgetComponent::GetFont() const {
        return font;
    }

    // Outline methods
    void TextWidgetComponent::SetOutlineEnabled(bool enabled) {
        outlineEnabled = enabled;
    }

    bool TextWidgetComponent::IsOutlineEnabled() const {
        return outlineEnabled;
    }

    void TextWidgetComponent::SetOutlineColor(const glm::vec4& color) {
        outlineColor = color;
    }

    glm::vec4 TextWidgetComponent::GetOutlineColor() const {
        return outlineColor;
    }

    void TextWidgetComponent::SetOutlineThickness(float thickness) {
        outlineThickness = thickness;
    }

    float TextWidgetComponent::GetOutlineThickness() const {
        return outlineThickness;
    }

    void TextWidgetComponent::SetShadowEnabled(bool enabled) {
        shadowEnabled = enabled;
    }

    bool TextWidgetComponent::IsShadowEnabled() const {
        return shadowEnabled;
    }

    void TextWidgetComponent::SetShadowColor(const glm::vec4& color) {
        shadowColor = color;
    }

    glm::vec4 TextWidgetComponent::GetShadowColor() const {
        return shadowColor;
    }

    void TextWidgetComponent::SetShadowOffset(const glm::vec2& offset) {
        shadowOffset = offset;
    }

    glm::vec2 TextWidgetComponent::GetShadowOffset() const {
        return shadowOffset;
    }

    void TextWidgetComponent::Start() {
        font = Engine::GetInstance().GetFontManager().GetFont("fonts/NotoSans.ttf", 32);
        spdlog::info("TextWidgetComponent::Start Default font set to 'NotoSans.ttf' with size 32");
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

        if (json.contains("outline")) {
            const auto& outlineObj = json["outline"];
            outlineEnabled         = outlineObj.value("enabled", false);
            outlineThickness       = outlineObj.value("thickness", 2.0f);

            if (outlineObj.contains("color")) {
                const auto& col = outlineObj["color"];
                outlineColor    = {col.value("r", 0.0f), col.value("g", 0.0f), col.value("b", 0.0f), col.value("a", 1.0f)};
            }
        }

        if (json.contains("shadow")) {
            const auto& shadowObj = json["shadow"];
            shadowEnabled         = shadowObj.value("enabled", false);

            if (shadowObj.contains("offset")) {
                const auto& off = shadowObj["offset"];
                shadowOffset    = {off.value("x", 2.0f), off.value("y", 2.0f)};
            }

            if (shadowObj.contains("color")) {
                const auto& col = shadowObj["color"];
                shadowColor     = {col.value("r", 0.0f), col.value("g", 0.0f), col.value("b", 0.0f), col.value("a", 0.5f)};
            }
        }
    }

    void TextWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (text.empty() || !font) {
            return;
        }

        auto pos          = GetPivotPos();
        auto& fontManager = Engine::GetInstance().GetFontManager();

        auto draw_text_internal = [&](const glm::vec2& basePos, const glm::vec4& color) {
            float cursorX   = basePos.x;
            float cursorY   = basePos.y;
            const char* ptr = text.c_str();
            const char* end = ptr + text.length();

            while (ptr < end) {
                uint32_t codepoint = DecodeUTF8(ptr);

                std::shared_ptr<Font> glyphFont;
                const Glyph* glyph = fontManager.GetGlyphWithFallback(font, codepoint, glyphFont);

                if (!glyph || !glyphFont) {
                    continue;
                }

                int glyphW = glyphFont->GetTexture()->GetWidth();
                int glyphH = glyphFont->GetTexture()->GetHeight();

                float x1 = cursorX;
                float y1 = cursorY - glyph->height + glyph->yOffset;
                float x2 = x1 + static_cast<float>(glyph->width);
                float y2 = y1 + static_cast<float>(glyph->height);

                float u1 = static_cast<float>(glyph->x0) / static_cast<float>(glyphW);
                float v1 = static_cast<float>(glyph->y0) / static_cast<float>(glyphH);
                float u2 = static_cast<float>(glyph->x1) / static_cast<float>(glyphW);
                float v2 = static_cast<float>(glyph->y1) / static_cast<float>(glyphH);

                cursorX += static_cast<float>(glyph->advance);

                pCanvas->DrawTexture2D(
                    glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec2(u1, v2), glm::vec2(u2, v1), glyphFont->GetTexture().get(), color);
            }
        };

        if (shadowEnabled) {
            draw_text_internal(pos + shadowOffset, shadowColor);
        }

        if (outlineEnabled) {
            glm::vec2 offsets[] = {
                {-outlineThickness, -outlineThickness},
                {0,                 -outlineThickness},
                {outlineThickness,  -outlineThickness},
                {-outlineThickness, 0                },
                {outlineThickness,  0                },
                {-outlineThickness, outlineThickness },
                {0,                 outlineThickness },
                {outlineThickness,  outlineThickness }
            };

            for (const auto& offset : offsets) {
                draw_text_internal(pos + offset, outlineColor);
            }
        }

        draw_text_internal(pos, textColor);
    }

    glm::vec2 TextWidgetComponent::GetPivotPos() const {
        auto pos = GetOwner()->GetWorldPosition2D();

        glm::vec2 rect(0.0f);

        const char* ptr = text.c_str();
        const char* end = ptr + text.length();

        auto& fontManager = Engine::GetInstance().GetFontManager();

        while (ptr < end) {
            uint32_t codepoint = DecodeUTF8(ptr);

            std::shared_ptr<Font> glyphFont;
            const Glyph* glyph = fontManager.GetGlyphWithFallback(font, codepoint, glyphFont);

            if (glyph) {
                rect.x += static_cast<float>(glyph->advance);
                rect.y = glm::max(rect.y, static_cast<float>(glyph->height));
            }
        }

        pos.x -= SDL_roundf(rect.x * pivot.x);
        pos.y -= SDL_roundf(rect.y * pivot.y);

        return pos;
    }


} // namespace golias
