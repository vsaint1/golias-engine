#include "scene/components/widget/input_slider_component.h"

#include "core/engine.h"
#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    bool InputSliderComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        if (properties.contains("min")) {
            mMinValue = properties["min"].get<float>();
        }

        if (properties.contains("max")) {
            mMaxValue = properties["max"].get<float>();
        }

        if (properties.contains("value")) {
            mValue = properties["value"].get<float>();
        }

        if (properties.contains("color")) {
            const Json& c = properties["color"];

            if (c.contains("track")) {
                const Json& track = c["track"];
                mTrackColor = glm::vec4(track.value("r", 0.25f), track.value("g", 0.25f), track.value("b", 0.25f), track.value("a", 1.0f));
            }

            if (c.contains("fill")) {
                const Json& fill = c["fill"];
                mFillColor       = glm::vec4(fill.value("r", 0.3f), fill.value("g", 0.6f), fill.value("b", 1.0f), fill.value("a", 1.0f));
            }

            if (c.contains("handle")) {
                const Json& handle = c["handle"];
                mHandleColor =
                    glm::vec4(handle.value("r", 0.9f), handle.value("g", 0.9f), handle.value("b", 0.9f), handle.value("a", 1.0f));
                *mCurrentHandleColor = mHandleColor;
            }
        }


        SetValue(mValue, false);

        return true;
    }

    void InputSliderComponent::Update(float deltaTime) {
        if (mDragging) {
            const InputManager& inputManager = Engine::GetInstance().GetInputManager();

            if (inputManager.IsMouseButtonPressed(MouseButton::Left)) {
                glm::vec2 mousePos = inputManager.GetMousePosition();
                float newValue     = ValueFromMouseX(mousePos.x);

                if (newValue != mValue) {
                    mValue = newValue;

                    if (onValueChanged) {
                        onValueChanged(mValue);
                    }
                }
            } else {
                mDragging           = false;
                mCurrentHandleColor = &mHandleColor;
            }
        }
    }

    void InputSliderComponent::Render(CanvasComponent* canvas) {
        if (!canvas) {
            return;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        const glm::vec2 size      = rectTransform->GetSize();
        const glm::vec2 origin    = screenPos - rectTransform->GetPivot() * size;

        const float trackHeight = size.y * kTrackHeightFraction;
        const float trackY      = origin.y + (size.y - trackHeight) * 0.5f;

        const glm::vec2 trackLowerLeft  = glm::vec2(origin.x, trackY);
        const glm::vec2 trackUpperRight = glm::vec2(origin.x + size.x, trackY + trackHeight);

        canvas->DrawQuad(trackLowerLeft, trackUpperRight, mTrackColor);

        const float t                  = NormalizeValue();
        const float fillX              = origin.x + size.x * t;
        const glm::vec2 fillLowerLeft  = glm::vec2(origin.x, trackY);
        const glm::vec2 fillUpperRight = glm::vec2(fillX, trackY + trackHeight);

        canvas->DrawQuad(fillLowerLeft, fillUpperRight, mFillColor);

        const float handleWidth   = size.x * kHandleWidthFraction;
        const float handleHeight  = trackHeight * kHandleHeightFraction;
        const float handleCenterX = origin.x + size.x * t;
        const float handleCenterY = trackY + trackHeight * 0.5f;

        const glm::vec2 handleLowerLeft  = glm::vec2(handleCenterX - handleWidth * 0.5f, handleCenterY - handleHeight * 0.5f);
        const glm::vec2 handleUpperRight = glm::vec2(handleCenterX + handleWidth * 0.5f, handleCenterY + handleHeight * 0.5f);

        canvas->DrawQuad(handleLowerLeft, handleUpperRight, *mCurrentHandleColor);
    }

    bool InputSliderComponent::HitTest(const glm::vec2& point) {
        if (!mIsEnabled) {
            return false;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return false;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        const glm::vec2 size      = rectTransform->GetSize();
        const glm::vec2 p1        = screenPos - rectTransform->GetPivot() * size;
        const glm::vec2 p2        = p1 + size;

        return (point.x >= p1.x && point.x <= p2.x && point.y >= p1.y && point.y <= p2.y);
    }

    void InputSliderComponent::OnPointerEnter() {
        mCurrentHandleColor = &mHoveredHandleColor;
    }

    void InputSliderComponent::OnPointerExit() {
        if (!mDragging) {
            mCurrentHandleColor = &mHandleColor;
        }
    }

    void InputSliderComponent::OnPointerDown() {
        mDragging           = true;
        mCurrentHandleColor = &mHoveredHandleColor;

        const InputManager& inputManager = Engine::GetInstance().GetInputManager();
        glm::vec2 mousePos               = inputManager.GetMousePosition();
        float newValue                   = ValueFromMouseX(mousePos.x);

        if (newValue != mValue) {
            mValue = newValue;

            if (onValueChanged) {
                onValueChanged(mValue);
            }
        }
    }

    void InputSliderComponent::OnPointerUp() {
        mDragging           = false;
        mCurrentHandleColor = &mHandleColor;
    }

    float InputSliderComponent::GetMinValue() const {
        return mMinValue;
    }

    void InputSliderComponent::SetMinValue(float min) {
        mMinValue = min;
        SetValue(mValue);
    }

    float InputSliderComponent::GetMaxValue() const {
        return mMaxValue;
    }

    void InputSliderComponent::SetMaxValue(float max) {
        mMaxValue = max;
        SetValue(mValue);
    }

    float InputSliderComponent::GetValue() const {
        return mValue;
    }

    void InputSliderComponent::SetValue(float value, bool notify) {
        mValue = glm::clamp(value, mMinValue, mMaxValue);

        if (notify && onValueChanged) {
            onValueChanged(mValue);
        }
    }

    const glm::vec4& InputSliderComponent::GetTrackColor() const {
        return mTrackColor;
    }

    void InputSliderComponent::SetTrackColor(const glm::vec4& color) {
        mTrackColor = color;
    }

    const glm::vec4& InputSliderComponent::GetFillColor() const {
        return mFillColor;
    }

    void InputSliderComponent::SetFillColor(const glm::vec4& color) {
        mFillColor = color;
    }

    const glm::vec4& InputSliderComponent::GetHandleColor() const {
        return mHandleColor;
    }

    void InputSliderComponent::SetHandleColor(const glm::vec4& color) {
        mHandleColor        = color;
        mCurrentHandleColor = &mHandleColor;
    }

    float InputSliderComponent::NormalizeValue() const {
        if (mMaxValue <= mMinValue) {
            return 0.0f;
        }

        return (mValue - mMinValue) / (mMaxValue - mMinValue);
    }

    float InputSliderComponent::ValueFromMouseX(float mouseX) const {
        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return mValue;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        const glm::vec2 size      = rectTransform->GetSize();
        const glm::vec2 origin    = screenPos - rectTransform->GetPivot() * size;

        float t = (mouseX - origin.x) / size.x;
        t       = glm::clamp(t, 0.0f, 1.0f);

        return mMinValue + t * (mMaxValue - mMinValue);
    }

} // namespace golias
