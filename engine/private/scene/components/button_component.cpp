#include "scene/components/widget/button_component.h"

#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {


    bool ButtonComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

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


        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return;
        }


        glm::vec2 pos = rectTransform->GetScreenPosition();
        pos -= rectTransform->GetPivot() * rectTransform->GetSize();

        if (!mIsEnabled) {
            mCurrentColor = &mDisabledColor;
        }

        canvas->DrawQuad(pos, pos + rectTransform->GetSize(), *mCurrentColor);
    }

    bool ButtonComponent::HitTest(const glm::vec2& point) {

        if(!mIsEnabled){
            return false;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return false;
        }


        glm::vec2 pos = rectTransform->GetScreenPosition();
        glm::vec2 p1  = pos - rectTransform->GetPivot() * rectTransform->GetSize();
        glm::vec2 p2  = p1 + rectTransform->GetSize();


        return (point.x >= p1.x && point.x <= p2.x && point.y >= p1.y && point.y <= p2.y);
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

    glm::vec4 ButtonComponent::GetColor() const {
        return mColor;
    }

    void ButtonComponent::SetColor(const glm::vec4& color) {
        mColor        = color;
        mCurrentColor = &mColor;
    }
} // namespace golias
