#pragma once

#include "widget_component.h"

namespace golias {

    class ProgressBarComponent : public WidgetComponent {
        COMPONENT_DERIVED(ProgressBarComponent, WidgetComponent)
    public:
        ProgressBarComponent()  = default;
        ~ProgressBarComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        bool HitTest(const glm::vec2& point) override;

        float GetMinValue() const;
        void SetMinValue(float min);

        float GetMaxValue() const;
        void SetMaxValue(float max);

        float GetValue() const;
        void SetValue(float value);

        const glm::vec4& GetBackgroundColor() const;
        void SetBackgroundColor(const glm::vec4& color);

        const glm::vec4& GetFillColor() const;
        void SetFillColor(const glm::vec4& color);

    private:
        float NormalizeValue() const;


        float mMinValue = 0.0f;
        float mMaxValue = 100.0f;
        float mValue    = 50.0f;

        glm::vec4 mBackgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 mFillColor       = glm::vec4(0.3f, 0.7f, 0.3f, 1.0f);
    };

} // namespace golias
