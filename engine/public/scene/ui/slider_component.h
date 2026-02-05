#pragma once
#include "scene/ui/ui_base.h"
#include <functional>
#include <glm/vec4.hpp>

namespace golias {

    /// @brief Slider direction
    enum class ESliderDirection {
        LEFT_TO_RIGHT,
        RIGHT_TO_LEFT,
        BOTTOM_TO_TOP,
        TOP_TO_BOTTOM
    };

    using SliderCallback = std::function<void(float)>;

    /// @brief  Slider component for value selection
    class SliderWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(SliderWidgetComponent, WidgetComponent)
    public:
        void SetValue(float value);
        float GetValue() const;

        void SetMinValue(float min);
        float GetMinValue() const;

        void SetMaxValue(float max);
        float GetMaxValue() const;

        void SetWholeNumbers(bool wholeNumbers);
        bool GetWholeNumbers() const;

        void SetDirection(ESliderDirection direction);
        ESliderDirection GetDirection() const;

        void SetBackgroundColor(const glm::vec4& color);
        glm::vec4 GetBackgroundColor() const;

        void SetFillColor(const glm::vec4& color);
        glm::vec4 GetFillColor() const;

        void SetHandleColor(const glm::vec4& color);
        glm::vec4 GetHandleColor() const;

        void SetHandleSize(const glm::vec2& size);
        glm::vec2 GetHandleSize() const;

        void SetInteractable(bool interactable);
        bool IsInteractable() const;

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;
        void OnPointerDown() override;
        void OnPointerUp() override;

        /// @brief Called when value changes
        SliderCallback OnValueChanged;

    private:
        float CalculateValueFromPosition(const glm::vec2& mousePos) const;

        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        bool wholeNumbers = false;
        
        ESliderDirection direction = ESliderDirection::LEFT_TO_RIGHT;
        
        glm::vec4 backgroundColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
        glm::vec4 fillColor = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);
        glm::vec4 handleColor = glm::vec4(1.0f);
        
        glm::vec2 handleSize = glm::vec2(20.0f, 30.0f);
        
        bool interactable = true;
        bool isDragging = false;
    };

} // namespace golias
