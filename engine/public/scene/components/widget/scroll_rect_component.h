#pragma once

#include "mask_component.h"

namespace golias {

    class CanvasComponent;

    class ScrollRectComponent : public MaskComponent {
        COMPONENT_DERIVED(ScrollRectComponent, MaskComponent)
    public:
        ScrollRectComponent()  = default;
        ~ScrollRectComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        bool HitTest(const glm::vec2& point) override;

        void OnPointerEnter() override;
        void OnPointerExit() override;

        void OnPointerDown() override;
        void OnPointerUp() override;

        glm::vec2 GetContentOffset() const override;

        float GetScrollOffset() const;
        void SetScrollOffset(float offset);

        float GetMaxScrollOffset() const;

        void SetContentLengthOverride(float length);

        bool IsVertical() const;

        const glm::vec4& GetColor() const;
        void SetColor(const glm::vec4& color);

        bool GetShowScrollbar() const;
        void SetShowScrollbar(bool show);

    private:
        bool GetViewportRect(glm::vec2& lowerLeft, glm::vec2& size) const;

        bool GetBarStripRect(glm::vec2& lowerLeft, glm::vec2& size) const;

        float GetHandleLength() const;

        float OffsetFromMouse(float mousePos) const;

        bool IsBarVisible() const;

    private:
        float mScrollOffset      = 0.0f;
        float mScrollSensitivity = 32.0f;

        float mContentLengthOverride = 0.0f;

        bool mVertical = true;

        glm::vec4 mColor = glm::vec4(0.45f, 0.45f, 0.45f, 1.0f);

        bool mShowScrollbar = false;

        bool mDragging = false;

        float mDragGrabOffset = 0.0f;

        static constexpr float kScrollbarThickness = 12.0f;
        static constexpr float kScrollbarMargin    = 2.0f;
        static constexpr float kMinScrollbarLength = 32.0f;

        glm::vec4 mHandleColor        = glm::vec4(0.45f, 0.45f, 0.45f, 0.8f);
        glm::vec4 mHoveredHandleColor = glm::vec4(0.6f, 0.6f, 0.6f, 0.9f);

        const glm::vec4* mCurrentHandleColor = &mHandleColor;
    };

} // namespace golias
