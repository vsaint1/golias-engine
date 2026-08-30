#include "scene/components/widget/button_component.h"

#include "scene/components/widget/canvas_component.h"
#include "scene/game_object.h"

namespace golias {


    bool ButtonComponent::LoadProperties(const Json& properties) {
        if (properties.contains("rect")) {
            const Json& rectObj = properties["rect"];

            glm::vec2 rectSize;
            rectSize.x = rectObj.value("x", 0.0f);
            rectSize.y = rectObj.value("y", 0.0f);

            SetRect(rectSize);
        }

        if (properties.contains("color")) {
            const Json& colorObj = properties["color"];
            const float r        = colorObj.value("r", 1.0f);
            const float g        = colorObj.value("g", 1.0f);
            const float b        = colorObj.value("b", 1.0f);
            const float a        = colorObj.value("a", 1.0f);

            glm::vec4 color = glm::vec4(r, g, b, a);

            SetColor(color);
        }

        return true;
    }

    void ButtonComponent::Update(float deltaTime) {
    }

    void ButtonComponent::Render(CanvasComponent* canvas) {

        if (!canvas) {
            return;
        }

        glm::vec2 pos = GetOwner()->GetWorldPosition2D();
        pos.x -= mRect.x * mPivot.x;
        pos.y -= mRect.y * mPivot.y;

        canvas->DrawQuad(pos, pos + mRect, *mCurrentColor);
    }

    bool ButtonComponent::HitTest(const glm::vec2& point) {
        glm::vec2 pos = GetOwner()->GetWorldPosition2D();

        float x1 = pos.x - mRect.x * mPivot.x;
        float y1 = pos.y - mRect.y * mPivot.y;
        float x2 = x1 + mRect.x;
        float y2 = y1 + mRect.y;

        return (point.x >= x1 && point.x <= x2 && point.y >= y1 && point.y <= y2);
    }


    void ButtonComponent::OnPointerEnter() {
        mCurrentColor = &mHoveredColor;
    }

    void ButtonComponent::OnPointerExit() {
        mCurrentColor = &mColor;
    }

    void ButtonComponent::OnPointerUp() {
        mCurrentColor = &mHoveredColor;
    }

    void ButtonComponent::OnPointerDown() {
        mCurrentColor = &mPressedColor;
    }

    void ButtonComponent::OnClick() {

        if (onClick) {
            onClick();
        }
    }

    glm::vec2 ButtonComponent::GetRect() const {
        return mRect;
    }

    void ButtonComponent::SetRect(const glm::vec2& rect) {
        mRect = rect;
    }

    glm::vec4 ButtonComponent::GetColor() const {
        return mColor;
    }

    void ButtonComponent::SetColor(const glm::vec4& color) {
        mColor        = color;
        mCurrentColor = &mColor;
    }
} // namespace golias
