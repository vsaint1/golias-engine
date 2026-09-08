#include "scene/components/widget/scroll_rect_component.h"

#include "core/engine.h"
#include "core/input/input_manager.h"
#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    bool ScrollRectComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        if (properties.contains("color")) {
            const Json& colorObj = properties["color"];
            const float r        = colorObj.value("r", 1.0f);
            const float g        = colorObj.value("g", 1.0f);
            const float b        = colorObj.value("b", 1.0f);
            const float a        = colorObj.value("a", 1.0f);

            SetColor(glm::vec4(r, g, b, a));
        }

        const String direction = properties.value("direction", "vertical");
        mVertical              = (direction == "vertical");

        mShowScrollbar = properties.value("visible", false);

        return true;
    }

    void ScrollRectComponent::Update(float deltaTime) {
        UNUSED_PARAMETER(deltaTime);

        const InputManager& inputManager = Engine::GetInstance().GetInputManager();
        const glm::vec2 mousePos         = inputManager.GetMousePosition();

        glm::vec2 viewportLowerLeft;
        glm::vec2 viewportSize;
        if (!GetViewportRect(viewportLowerLeft, viewportSize)) {
            return;
        }

        if (mDragging) {
            if (inputManager.IsMouseButtonPressed(MouseButton::Left)) {
                SetScrollOffset(OffsetFromMouse(mVertical ? mousePos.y : mousePos.x));
            } else {
                mDragging           = false;
                mCurrentHandleColor = &mHandleColor;
            }

            return;
        }

        if (GetMaxScrollOffset() <= 0.0f) {
            return;
        }

        const float scroll = inputManager.GetScrollOffset().y;

        if (scroll != 0.0f && mousePos.x >= viewportLowerLeft.x && mousePos.x <= viewportLowerLeft.x + viewportSize.x
            && mousePos.y >= viewportLowerLeft.y && mousePos.y <= viewportLowerLeft.y + viewportSize.y) {
            mScrollOffset -= scroll * mScrollSensitivity;
            mScrollOffset = glm::clamp(mScrollOffset, 0.0f, GetMaxScrollOffset());
        }
    }

    void ScrollRectComponent::Render(CanvasComponent* canvas) {
        if (!canvas) {
            return;
        }

        glm::vec2 lowerLeft;
        glm::vec2 size;
        if (!GetViewportRect(lowerLeft, size)) {
            return;
        }

        canvas->DrawQuad(lowerLeft, lowerLeft + size, mColor);

        if (IsBarVisible()) {
            const float maxScroll = GetMaxScrollOffset();
            const float handleLen = GetHandleLength();

            if (mVertical) {
                const float travel = std::max(size.y - handleLen, 0.0f);
                const float t      = glm::clamp(mScrollOffset / maxScroll, 0.0f, 1.0f);

                const float barX = lowerLeft.x + size.x - kScrollbarMargin - kScrollbarThickness;
                const float barY = lowerLeft.y + travel * t;

                canvas->DrawQuad(glm::vec2(barX, barY), glm::vec2(barX + kScrollbarThickness, barY + handleLen), *mCurrentHandleColor);
            } else {
                const float travel = std::max(size.x - handleLen, 0.0f);
                const float t      = glm::clamp(mScrollOffset / maxScroll, 0.0f, 1.0f);

                const float barX = lowerLeft.x + travel * t;
                const float barY = lowerLeft.y + size.y - kScrollbarMargin - kScrollbarThickness;

                canvas->DrawQuad(glm::vec2(barX, barY), glm::vec2(barX + handleLen, barY + kScrollbarThickness), *mCurrentHandleColor);
            }
        }
    }

    bool ScrollRectComponent::HitTest(const glm::vec2& point) {
        if (!mIsEnabled || !IsBarVisible()) {
            return false;
        }

        glm::vec2 lowerLeft;
        glm::vec2 size;
        if (!GetBarStripRect(lowerLeft, size)) {
            return false;
        }

        return (point.x >= lowerLeft.x && point.x <= lowerLeft.x + size.x && point.y >= lowerLeft.y && point.y <= lowerLeft.y + size.y);
    }

    void ScrollRectComponent::OnPointerEnter() {
        mCurrentHandleColor = &mHoveredHandleColor;
    }

    void ScrollRectComponent::OnPointerExit() {
        if (!mDragging) {
            mCurrentHandleColor = &mHandleColor;
        }
    }

    void ScrollRectComponent::OnPointerDown() {
        mDragging           = true;
        mCurrentHandleColor = &mHoveredHandleColor;

        const InputManager& inputManager = Engine::GetInstance().GetInputManager();
        const float mousePos             = mVertical ? inputManager.GetMousePosition().y : inputManager.GetMousePosition().x;

        glm::vec2 lowerLeft;
        glm::vec2 size;
        if (!GetViewportRect(lowerLeft, size) || GetMaxScrollOffset() <= 0.0f) {
            return;
        }

        const float handleLen   = GetHandleLength();
        const float maxScroll   = GetMaxScrollOffset();
        const float viewportLen = mVertical ? size.y : size.x;
        const float handleStart = (mVertical ? lowerLeft.y : lowerLeft.x)
                                + std::max(viewportLen - handleLen, 0.0f) * glm::clamp(mScrollOffset / maxScroll, 0.0f, 1.0f);

        if (mousePos >= handleStart && mousePos <= handleStart + handleLen) {
            mDragGrabOffset = handleStart - mousePos;
        } else {
            mDragGrabOffset = -handleLen * 0.5f;
        }

        SetScrollOffset(OffsetFromMouse(mousePos));
    }

    void ScrollRectComponent::OnPointerUp() {
        mDragging           = false;
        mCurrentHandleColor = &mHandleColor;
    }

    glm::vec2 ScrollRectComponent::GetContentOffset() const {
        if (mVertical) {
            return glm::vec2(0.0f, -mScrollOffset);
        }

        return glm::vec2(-mScrollOffset, 0.0f);
    }

    float ScrollRectComponent::GetScrollOffset() const {
        return mScrollOffset;
    }

    void ScrollRectComponent::SetScrollOffset(float offset) {
        mScrollOffset = std::clamp(offset, 0.0f, GetMaxScrollOffset());
    }

    float ScrollRectComponent::GetMaxScrollOffset() const {
        glm::vec2 viewportLowerLeft;
        glm::vec2 viewportSize;
        if (!GetViewportRect(viewportLowerLeft, viewportSize)) {
            return 0.0f;
        }

        float contentExtent = 0.0f;

        if (mContentLengthOverride > 0.0f) {
            contentExtent = mContentLengthOverride;
        } else {
            for (const auto& child : GetOwner()->GetChildren()) {
                if (!child->IsActive()) {
                    continue;
                }

                if (RectTransformComponent* rectTransform = child->GetComponent<RectTransformComponent>()) {
                    const glm::vec2 pos  = rectTransform->GetScreenPosition() - rectTransform->GetPivot() * rectTransform->GetSize();
                    const glm::vec2 size = rectTransform->GetSize();

                    if (mVertical) {
                        contentExtent = std::max(contentExtent, pos.y + size.y - viewportLowerLeft.y);
                    } else {
                        contentExtent = std::max(contentExtent, pos.x + size.x - viewportLowerLeft.x);
                    }
                }
            }
        }

        const float viewportExtent = mVertical ? viewportSize.y : viewportSize.x;
        return std::max(contentExtent - viewportExtent, 0.0f);
    }

    void ScrollRectComponent::SetContentLengthOverride(float length) {
        mContentLengthOverride = length;

        SetScrollOffset(mScrollOffset);
    }

    bool ScrollRectComponent::GetBarStripRect(glm::vec2& lowerLeft, glm::vec2& size) const {
        if (!GetViewportRect(lowerLeft, size)) {
            return false;
        }

        if (mVertical) {
            lowerLeft.x = lowerLeft.x + size.x - kScrollbarMargin - kScrollbarThickness;
            size.x      = kScrollbarThickness;
        } else {
            lowerLeft.y = lowerLeft.y + size.y - kScrollbarMargin - kScrollbarThickness;
            size.y      = kScrollbarThickness;
        }

        return true;
    }

    float ScrollRectComponent::GetHandleLength() const {
        glm::vec2 viewportLowerLeft;
        glm::vec2 viewportSize;
        if (!GetViewportRect(viewportLowerLeft, viewportSize)) {
            return 0.0f;
        }

        const float viewportLen = std::max(mVertical ? viewportSize.y : viewportSize.x, 1.0f);
        const float contentLen  = std::max(mContentLengthOverride > 0.0f ? mContentLengthOverride : viewportLen, viewportLen);

        float handleLength = viewportLen * (viewportLen / contentLen);
        handleLength       = std::min(std::max(handleLength, kMinScrollbarLength), viewportLen);

        return handleLength;
    }

    float ScrollRectComponent::OffsetFromMouse(float mousePos) const {
        glm::vec2 lowerLeft;
        glm::vec2 size;
        if (!GetViewportRect(lowerLeft, size)) {
            return mScrollOffset;
        }

        const float maxScroll = GetMaxScrollOffset();
        if (maxScroll <= 0.0f) {
            return 0.0f;
        }

        const float viewportLen = mVertical ? size.y : size.x;
        const float trackStart  = mVertical ? lowerLeft.y : lowerLeft.x;
        const float travel      = std::max(viewportLen - GetHandleLength(), 0.0f);

        float t = (mousePos + mDragGrabOffset - trackStart) / travel;
        t       = glm::clamp(t, 0.0f, 1.0f);

        return t * maxScroll;
    }

    bool ScrollRectComponent::IsBarVisible() const {
        return mShowScrollbar && GetMaxScrollOffset() > 0.0f;
    }

    bool ScrollRectComponent::IsVertical() const {
        return mVertical;
    }

    const glm::vec4& ScrollRectComponent::GetColor() const {
        return mColor;
    }

    void ScrollRectComponent::SetColor(const glm::vec4& color) {
        mColor = color;
    }

    bool ScrollRectComponent::GetShowScrollbar() const {
        return mShowScrollbar;
    }

    void ScrollRectComponent::SetShowScrollbar(bool show) {
        mShowScrollbar = show;
    }

    bool ScrollRectComponent::GetViewportRect(glm::vec2& lowerLeft, glm::vec2& size) const {
        return GetMaskRect(lowerLeft, size);
    }

} // namespace golias
