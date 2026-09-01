#include "scene/components/widget/progress_bar_component.h"

#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    bool ProgressBarComponent::LoadProperties(const Json& properties) {
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

            if (c.contains("background")) {
                const Json& background = c["background"];
                mBackgroundColor = glm::vec4(background.value("r", 0.25f), background.value("g", 0.25f), background.value("b", 0.25f), background.value("a", 1.0f));
            }

            if (c.contains("fill")) {
                const Json& fill = c["fill"];
                mFillColor       = glm::vec4(fill.value("r", 0.3f), fill.value("g", 0.6f), fill.value("b", 1.0f), fill.value("a", 1.0f));
            }
          
        }

        SetValue(mValue);

        return true;
    }

    void ProgressBarComponent::Update(float deltaTime) {
    }

    void ProgressBarComponent::Render(CanvasComponent* canvas) {
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

        canvas->DrawQuad(origin, origin + size, mBackgroundColor);

        const float t = NormalizeValue();

        if (t > 0.0f) {
            const float fillWidth = size.x * t;

            const glm::vec2 fillLowerLeft  = glm::vec2(origin.x, origin.y);
            const glm::vec2 fillUpperRight = glm::vec2(origin.x + fillWidth, origin.y + size.y);

            canvas->DrawQuad(fillLowerLeft, fillUpperRight, mFillColor);
        }
    }

    bool ProgressBarComponent::HitTest(const glm::vec2& point) {
        return false;
    }

    float ProgressBarComponent::GetMinValue() const {
        return mMinValue;
    }

    void ProgressBarComponent::SetMinValue(float min) {
        mMinValue = min;
        SetValue(mValue);
    }

    float ProgressBarComponent::GetMaxValue() const {
        return mMaxValue;
    }

    void ProgressBarComponent::SetMaxValue(float max) {
        mMaxValue = max;
        SetValue(mValue);
    }

    float ProgressBarComponent::GetValue() const {
        return mValue;
    }

    void ProgressBarComponent::SetValue(float value) {
        mValue = glm::clamp(value, mMinValue, mMaxValue);
    }

    const glm::vec4& ProgressBarComponent::GetBackgroundColor() const {
        return mBackgroundColor;
    }

    void ProgressBarComponent::SetBackgroundColor(const glm::vec4& color) {
        mBackgroundColor = color;
    }

    const glm::vec4& ProgressBarComponent::GetFillColor() const {
        return mFillColor;
    }

    void ProgressBarComponent::SetFillColor(const glm::vec4& color) {
        mFillColor = color;
    }

    float ProgressBarComponent::NormalizeValue() const {
        if (mMaxValue <= mMinValue) {
            return 0.0f;
        }

        return glm::clamp((mValue - mMinValue) / (mMaxValue - mMinValue), 0.0f, 1.0f);
    }

} // namespace golias
