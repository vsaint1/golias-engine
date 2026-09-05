#pragma once

#include "widget_component.h"

namespace golias {

    class Font;
    class Texture2D;

    class DropdownComponent : public WidgetComponent {
        COMPONENT_DERIVED(DropdownComponent, WidgetComponent)
    public:
        DropdownComponent()  = default;
        ~DropdownComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        bool HitTest(const glm::vec2& point) override;

        void OnPointerEnter() override;
        void OnPointerExit() override;

        void OnPointerUp() override;
        void OnPointerDown() override;

        void OnClick() override;

        bool IsTopmost() const override;

        const std::vector<String>& GetOptions() const;
        void SetOptions(const std::vector<String>& options);

        int GetSelectedIndex() const;
        void SetSelectedIndex(int index, bool notify = true);

        const String& GetSelectedOption() const;

        bool IsOpen() const;
        void SetOpen(bool open);

        std::function<void(int)> onValueChanged;

    private:
        bool GetHeaderRect(glm::vec2& lowerLeft, glm::vec2& size) const;

        float GetRowHeight() const;

        int RowAt(const glm::vec2& point) const;

    private:
        static constexpr float kListGap        = 2.0f;
        static constexpr float kRowPadding     = 4.0f;
        static constexpr float kHeaderPaddingX = 10.0f;
        static constexpr float kArrowInsetX    = 20.0f;

        std::vector<String> mOptions;
        int mSelectedIndex = 0;

        bool mOpen      = false;
        int mHoveredRow = -1;

        String mFontPath;
        int mFontSize   = 16;
        Ref<Font> mFont = nullptr;

        Ref<Texture2D> mArrow = nullptr;

        glm::vec4 mColor            = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 mHoveredColor     = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 mPressedColor     = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
        glm::vec4 mDisabledColor    = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
        glm::vec4 mListColor        = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
        glm::vec4 mRowHoveredColor  = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        glm::vec4 mSelectedRowColor = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
        glm::vec4 mTextColor        = glm::vec4(1.0f);

        glm::vec4* mCurrentColor = &mColor;
    };

} // namespace golias
