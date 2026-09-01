#pragma once

#include "widget_component.h"

namespace golias {

    class InputSliderComponent : public WidgetComponent {
        COMPONENT_DERIVED(InputSliderComponent, WidgetComponent)
    public:
        InputSliderComponent()  = default;
        ~InputSliderComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        bool HitTest(const glm::vec2& point) override;

        void OnPointerEnter() override;
        void OnPointerExit() override;

        void OnPointerUp() override;
        void OnPointerDown() override;

        float GetMinValue() const;
        void SetMinValue(float min);

        float GetMaxValue() const;
        void SetMaxValue(float max);

        float GetValue() const;
        void SetValue(float value, bool notify = true);

        const glm::vec4& GetTrackColor() const;
        void SetTrackColor(const glm::vec4& color);

        const glm::vec4& GetFillColor() const;
        void SetFillColor(const glm::vec4& color);

        const glm::vec4& GetHandleColor() const;
        void SetHandleColor(const glm::vec4& color);

        std::function<void(float)> onValueChanged;

    private:
        float NormalizeValue() const;
        float ValueFromMouseX(float mouseX) const;

        static constexpr float kHandleWidthFraction  = 0.04f;
        static constexpr float kHandleHeightFraction = 1.6f;
        static constexpr float kTrackHeightFraction  = 0.3f;

        float mMinValue = 0.0f;
        float mMaxValue = 1.0f;
        float mValue    = 0.5f;

        bool mDragging = false;

        glm::vec4 mTrackColor         = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
        glm::vec4 mFillColor          = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
        glm::vec4 mHandleColor        = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
        glm::vec4 mHoveredHandleColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        glm::vec4* mCurrentHandleColor = &mHandleColor;
    };

} // namespace golias
