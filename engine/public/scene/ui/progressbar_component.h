#pragma once
#include "scene/ui/ui_base.h"
#include <glm/vec4.hpp>

namespace golias {

    /// @brief Progress Bar / Loading bar widget
    class ProgressBarWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(ProgressBarWidgetComponent, WidgetComponent)
    public:
        void SetValue(float value);
        float GetValue() const;

        void SetMinValue(float min);
        float GetMinValue() const;

        void SetMaxValue(float max);
        float GetMaxValue() const;

        void SetBackgroundColor(const glm::vec4& color);
        glm::vec4 GetBackgroundColor() const;

        void SetFillColor(const glm::vec4& color);
        glm::vec4 GetFillColor() const;

        /// @brief Set fill direction (left-to-right, right-to-left, bottom-to-top, top-to-bottom)
        void SetFillDirection(int direction);
        int GetFillDirection() const;

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

    private:
        float value = 0.5f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        
        glm::vec4 backgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 fillColor = glm::vec4(0.2f, 0.7f, 0.2f, 1.0f);
        
        int fillDirection = 0; // 0 = left-to-right, 1 = right-to-left, 2 = bottom-to-top, 3 = top-to-bottom
    };

} // namespace golias
