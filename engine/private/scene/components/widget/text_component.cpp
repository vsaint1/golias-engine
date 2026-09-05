#include "scene/components/widget/text_component.h"

#include "core/engine.h"
#include "font/font.h"
#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"

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

            // // TODO: We need handle pivot for text alignment and positioning.
        }

        if (properties.contains("effects") && properties["effects"].is_object()) {
            const Json& effectsObj = properties["effects"];

            if (effectsObj.contains("outline") && effectsObj["outline"].is_object()) {
                const Json& outlineObj = effectsObj["outline"];
                mHasOutline            = outlineObj.value("enabled", false);

                if (outlineObj.contains("color")) {
                    const Json& colorObj = outlineObj["color"];
                    const float r        = colorObj.value("r", 0.0f);
                    const float g        = colorObj.value("g", 0.0f);
                    const float b        = colorObj.value("b", 0.0f);
                    const float a        = colorObj.value("a", 1.0f);

                    mOutlineColor = glm::vec4(r, g, b, a);
                }
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

        const glm::vec4* outlineColor = mHasOutline ? &mOutlineColor : nullptr;

        canvas->DrawText(mFont.get(), GetPivot(), mText, mColor, outlineColor);
    }


    glm::vec2 TextComponent::GetPivot() const {

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();

        glm::vec2 pos = rectTransform ? rectTransform->GetScreenPosition() : GetOwner()->GetWorldPosition2D();

        const glm::vec2 rect = Measure();

        if (rectTransform) {
            pos -= rect * rectTransform->GetPivot();
        }

        return pos;
    }

    glm::vec2 TextComponent::GetDesiredSize() const {
        return Measure();
    }

    glm::vec2 TextComponent::Measure() const {
        if (!mFont) {
            return glm::vec2(0.0f);
        }

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

        return rect;
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
