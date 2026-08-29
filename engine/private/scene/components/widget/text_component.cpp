#include "scene/components/widget/text_component.h"

#include "core/engine.h"
#include "font/font.h"
#include "scene/components/widget/canvas_component.h"

namespace golias {

    bool TextComponent::LoadProperties(const Json& properties) {

        if (properties.contains("font") && properties["font"].is_object()) {

            const Json& fontObj = properties["font"];
            const String text   = fontObj.value("text", "");

            SetText(text);

            const String fontPath = fontObj.value("path", "");
            const int fontSize    = fontObj.value("size", 16);

            if (!fontPath.empty()) {
                mFontPath = fontPath;
                mFontSize = fontSize;
                SetFont(Engine::GetInstance().GetAssetManager().Load<Font>(fontPath.c_str(), fontSize));
            }

            if (fontObj.contains("color")) {
                const Json& colorObj = fontObj["color"];
                const float r        = colorObj.value("r", 1.0f);
                const float g        = colorObj.value("g", 1.0f);
                const float b        = colorObj.value("b", 1.0f);
                const float a        = colorObj.value("a", 1.0f);

                glm::vec4 color = glm::vec4(r, g, b, a);

                SetColor(color);
            }

            // TODO: We need handle pivot for text alignment and positioning.
            if (fontObj.contains("pivot")) {
                const Json& pivot  = fontObj["pivot"];
                const float x      = pivot.value("x", 0.0f);
                const float y      = pivot.value("y", 0.0f);
                
                SetPivot(glm::vec2(x, y));
            }
        }


        return true;
    }

    void TextComponent::Update(float deltaTime) {
    }

    void TextComponent::Render(CanvasComponent* canvas) {
        if (mText.empty() || !mFont || !canvas) {
            return;
        }

        const TextureDesc& texDesc = mFont->GetTexture()->GetDesc();
        const float invWidth       = 1.0f / static_cast<float>(texDesc.Width);
        const float invHeight      = 1.0f / static_cast<float>(texDesc.Height);

        const glm::vec2 origin    = GetPivot();
        const float lineHeight    = static_cast<float>(mFont->GetLineHeight());
        const float baseBaselineY = origin.y + static_cast<float>(mFont->GetSize());

        float cursorX   = origin.x;
        float baselineY = baseBaselineY;

        for (size_t i = 0; i < mText.size(); ++i) {
            const char c = mText[i];

            if (c == '\n') {
                cursorX = origin.x;
                baselineY += lineHeight;
                continue;
            }

            if (c == '\r') {
                if (i + 1 < mText.size() && mText[i + 1] == '\n') {
                    continue;
                }

                cursorX = origin.x;
                continue;
            }

            const auto& desc = mFont->GetGlyphDescription(c);

            const float x1 = cursorX + static_cast<float>(desc.OffsetX);
            const float y1 = baselineY + static_cast<float>(desc.OffsetY);
            const float x2 = x1 + static_cast<float>(desc.Width);
            const float y2 = y1 + static_cast<float>(desc.Height);

            const float u1 = (static_cast<float>(desc.X0)) * invWidth;
            const float v1 = (static_cast<float>(desc.Y0)) * invHeight;
            const float u2 = (static_cast<float>(desc.X1)) * invWidth;
            const float v2 = (static_cast<float>(desc.Y1)) * invHeight;

            cursorX += static_cast<float>(desc.Advance);

            canvas->DrawQuad(glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec2(u1, v1), glm::vec2(u2, v2), mFont->GetTexture().get(), mColor);
        }
    }


    glm::vec2 TextComponent::GetPivot() const {

        glm::vec2 pos = GetOwner()->GetWorldPosition2D();

        const float lineHeight = static_cast<float>(mFont->GetLineHeight());

        glm::vec2 rect(0.0f);
        float lineWidth     = 0.0f;
        float lineMaxHeight = 0.0f;
        int lineCount       = 1;

        for (size_t i = 0; i < mText.size(); ++i) {
            const char c = mText[i];

            if (c == '\n') {
                rect.y        = std::max(rect.y, lineMaxHeight);
                lineWidth     = 0.0f;
                lineMaxHeight = 0.0f;
                lineCount += 1;
                continue;
            }

            if (c == '\r') {
                if (i + 1 < mText.size() && mText[i + 1] == '\n') {
                    continue;
                }

                rect.y        = std::max(rect.y, lineMaxHeight);
                lineWidth     = 0.0f;
                lineMaxHeight = 0.0f;
                lineCount += 1;
                continue;
            }

            const auto& desc = mFont->GetGlyphDescription(c);

            lineWidth += static_cast<float>(desc.Advance);
            lineMaxHeight = std::max(lineMaxHeight, static_cast<float>(desc.Height));

            rect.x = std::max(rect.x, lineWidth);
        }

        rect.y = std::max(rect.y, lineMaxHeight);
        rect.y += static_cast<float>(lineCount - 1) * lineHeight;

        pos.x -= std::round(rect.x * mPivot.x);
        pos.y -= std::round(rect.y * mPivot.y);

        return pos;
    }

    void TextComponent::SetPivot(const glm::vec2& pivot) {
        mPivot = pivot;
    }


    const String& TextComponent::GetText() const {
        return mText;
    }

    void TextComponent::SetText(const String& text) {
        mText = text;
    }

    Ref<Font> TextComponent::GetFont() const {
        return mFont;
    }

    void TextComponent::SetFont(const Ref<Font>& font) {
        mFont = font;
    }

    int TextComponent::GetFontSize() const {
        return mFontSize;
    }

    void TextComponent::SetFontSize(int size) {
        if (size <= 0) {
            return;
        }

        mFontSize = size;

        if (!mFontPath.empty()) {
            Ref<Font> font = Engine::GetInstance().GetAssetManager().Load<Font>(mFontPath.c_str(), mFontSize);
            SetFont(font);
        }
    }

    const glm::vec4& TextComponent::GetColor() const {
        return mColor;
    }

    void TextComponent::SetColor(const glm::vec4& color) {
        mColor = color;
    }
} // namespace golias
