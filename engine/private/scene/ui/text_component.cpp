#include "scene/ui/text_component.h"

#include "core/engine.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

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
        font = Engine::GetInstance().GetAssetManager().EnsureFont(pFilePath, size);

        if (!font) {
            spdlog::error("TextWidgetComponent::SetFont Failed to load font '{}' with size {}", pFilePath, size);
        }
    }

    void TextWidgetComponent::SetFontSize(int size) {
        fontSize = size;

        if (font) {
            font->SetSize(size);
        }
    }

    int TextWidgetComponent::GetFontSize() const {
        return fontSize;
    }

    const std::shared_ptr<Font>& TextWidgetComponent::GetFont() const {
        return font;
    }

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
        font = Engine::GetInstance().GetAssetManager().EnsureFont("fonts/NotoSans.ttf", 32);
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

        auto pos           = GetPivotPos();
        auto& assetManager = Engine::GetInstance().GetAssetManager();
        float lineHeight   = static_cast<float>(font->GetSize());
        
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        lineHeight *= uiScale;

        auto draw_text_internal = [&](const glm::vec2& basePos, const glm::vec4& color, float zOffset) {
            float cursorX   = basePos.x;
            float cursorY   = basePos.y;
            const char* ptr = text.c_str();
            const char* end = ptr + text.length();

            while (ptr < end) {
                uint32_t codepoint = DecodeUTF8(ptr);

                if (codepoint == '\n') {
                    cursorX = basePos.x;
                    cursorY -= lineHeight;
                    continue;
                }

                if (codepoint == '\r') {
                    cursorX = basePos.x;
                    continue;
                }

                std::shared_ptr<Font> glyphFont;
                const Glyph* glyph = assetManager.GetGlyphWithFallback(font, codepoint, glyphFont);

                if (!glyph || !glyphFont) {
                    continue;
                }

                int glyphW = glyphFont->GetTexture()->GetWidth();
                int glyphH = glyphFont->GetTexture()->GetHeight();

                float x1 = cursorX;
                float y1 = cursorY - glyph->height * uiScale + glyph->yOffset * uiScale;
                float x2 = x1 + static_cast<float>(glyph->width) * uiScale;
                float y2 = y1 + static_cast<float>(glyph->height) * uiScale;

                float u1 = static_cast<float>(glyph->x0) / static_cast<float>(glyphW);
                float v1 = static_cast<float>(glyph->y0) / static_cast<float>(glyphH);
                float u2 = static_cast<float>(glyph->x1) / static_cast<float>(glyphW);
                float v2 = static_cast<float>(glyph->y1) / static_cast<float>(glyphH);

                cursorX += static_cast<float>(glyph->advance) * uiScale;

                pCanvas->DrawTexture2D(glm::vec3(x1, y1, zOffset),
                                       glm::vec3(x2, y2, zOffset),
                                       glm::vec2(u1, v2),
                                       glm::vec2(u2, v1),
                                       glyphFont->GetTexture().get(),
                                       color);
            }
        };

        if (shadowEnabled) {
            draw_text_internal(pos + shadowOffset * uiScale, shadowColor, -0.002f);
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
                draw_text_internal(pos + offset * uiScale, outlineColor, -0.001f);
            }
        }

        draw_text_internal(pos, textColor, 0.0f);
    }

    glm::vec2 TextWidgetComponent::GetPivotPos() const {

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        glm::vec2 textSize = MeasureText();

        if (rt) {
           
            auto parent = GetOwner()->GetParent();
            if (!parent || !parent->GetComponent<RectTransformComponent>()) {
                return GetOwner()->GetPosition2D();
            }
            
            float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();

            auto parentRect = parent->GetComponent<RectTransformComponent>();
            glm::vec2 parentPos = parentRect->GetScreenPosition();
            
           
            bool parentIsCanvas = parent->GetComponent<CanvasComponent>() != nullptr;
            
            glm::vec2 parentSize = parentRect->GetSize();
            if (!parentIsCanvas) {
                parentSize *= uiScale;
            }
            
            glm::vec2 anchorPos = parentPos + rt->GetAnchor() * parentSize;
            
            glm::vec2 localPos = GetOwner()->GetPosition2D() * uiScale; 
            glm::vec2 textPivotOffset = rt->GetPivot() * textSize * uiScale;
            
            return anchorPos + localPos - textPivotOffset;
        } else {
            // No RectTransform - use world position and center the text
            auto pos = GetOwner()->GetWorldPosition2D();
            pos.x -= SDL_roundf(textSize.x * 0.5f);
            pos.y -= SDL_roundf(textSize.y * 0.5f);
            return pos;
        }
    }

    glm::vec2 TextWidgetComponent::MeasureText() const {
        if (text.empty() || !font) {
            return glm::vec2(0.0f);
        }

        glm::vec2 size(0.0f);
        float currentLineWidth = 0.0f;
        float lineHeight = static_cast<float>(font->GetSize());
        int lineCount = 1;

        const char* ptr = text.c_str();
        const char* end = ptr + text.length();

        auto& assetManager = Engine::GetInstance().GetAssetManager();

        while (ptr < end) {
            uint32_t codepoint = DecodeUTF8(ptr);

            if (codepoint == '\n') {
                size.x = glm::max(size.x, currentLineWidth);
                currentLineWidth = 0.0f;
                lineCount++;
                continue;
            }

            if (codepoint == '\r') {
                size.x = glm::max(size.x, currentLineWidth);
                currentLineWidth = 0.0f;
                continue;
            }

            std::shared_ptr<Font> glyphFont;
            const Glyph* glyph = assetManager.GetGlyphWithFallback(font, codepoint, glyphFont);

            if (glyph) {
                currentLineWidth += static_cast<float>(glyph->advance);
            }
        }

        size.x = glm::max(size.x, currentLineWidth);
        size.y = lineHeight * lineCount;

        return size;
    }


} // namespace golias
