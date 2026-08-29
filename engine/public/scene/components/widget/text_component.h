#pragma once

#include "widget_component.h"

namespace golias {

    class Font;

    class TextComponent : public WidgetComponent {
        COMPONENT_DERIVED(TextComponent, WidgetComponent)
    public:
        TextComponent()  = default;
        ~TextComponent() = default;

        bool LoadProperties(const Json& properties);

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        const String& GetText() const;
        void SetText(const String& text);

        const glm::vec4& GetColor() const;
        void SetColor(const glm::vec4& color);

        Ref<Font> GetFont() const;
        void SetFont(const Ref<Font>& font);

        int GetFontSize() const;
        void SetFontSize(int size);

        glm::vec2 GetPivot() const;
        void SetPivot(const glm::vec2& pivot);

    private:
        String mText;
        String mFontPath;

        glm::vec4 mColor = glm::vec4(1.0f);
        Ref<Font> mFont  = nullptr;
        int mFontSize    = 16;
    };
} // namespace golias
