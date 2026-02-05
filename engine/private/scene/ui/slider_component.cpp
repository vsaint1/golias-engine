#include "scene/ui/slider_component.h"

#include "core/engine.h"
#include "core/input/input_manager.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void SliderWidgetComponent::SetValue(float val) {
        float oldValue = value;
        value = glm::clamp(val, minValue, maxValue);
        
        if (wholeNumbers) {
            value = std::round(value);
        }
        
        if (value != oldValue && OnValueChanged) {
            OnValueChanged(value);
        }
    }

    float SliderWidgetComponent::GetValue() const {
        return value;
    }

    void SliderWidgetComponent::SetMinValue(float min) {
        minValue = min;
        SetValue(value); // Re-clamp
    }

    float SliderWidgetComponent::GetMinValue() const {
        return minValue;
    }

    void SliderWidgetComponent::SetMaxValue(float max) {
        maxValue = max;
        SetValue(value); // Re-clamp
    }

    float SliderWidgetComponent::GetMaxValue() const {
        return maxValue;
    }

    void SliderWidgetComponent::SetWholeNumbers(bool whole) {
        wholeNumbers = whole;
        if (wholeNumbers) {
            SetValue(std::round(value));
        }
    }

    bool SliderWidgetComponent::GetWholeNumbers() const {
        return wholeNumbers;
    }

    void SliderWidgetComponent::SetDirection(ESliderDirection dir) {
        direction = dir;
    }

    ESliderDirection SliderWidgetComponent::GetDirection() const {
        return direction;
    }

    void SliderWidgetComponent::SetBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }

    glm::vec4 SliderWidgetComponent::GetBackgroundColor() const {
        return backgroundColor;
    }

    void SliderWidgetComponent::SetFillColor(const glm::vec4& color) {
        fillColor = color;
    }

    glm::vec4 SliderWidgetComponent::GetFillColor() const {
        return fillColor;
    }

    void SliderWidgetComponent::SetHandleColor(const glm::vec4& color) {
        handleColor = color;
    }

    glm::vec4 SliderWidgetComponent::GetHandleColor() const {
        return handleColor;
    }

    void SliderWidgetComponent::SetHandleSize(const glm::vec2& size) {
        handleSize = size;
    }

    glm::vec2 SliderWidgetComponent::GetHandleSize() const {
        return handleSize;
    }

    void SliderWidgetComponent::SetInteractable(bool val) {
        interactable = val;
    }

    bool SliderWidgetComponent::IsInteractable() const {
        return interactable;
    }

    void SliderWidgetComponent::Start() {
    }

    void SliderWidgetComponent::Update(float deltaTime) {
        if (!interactable || !isDragging) {
            return;
        }

        auto& input = Engine::GetInstance().GetInputManager();
        glm::vec2 mousePos = input.GetMousePosition();
        
        // Flip Y for screen coordinates
        auto& renderer = Engine::GetInstance().GetSceneRenderer();
        const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
        mousePos.y = viewport.height - mousePos.y;

        float newValue = CalculateValueFromPosition(mousePos);
        SetValue(newValue);
    }

    void SliderWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (!pCanvas) {
            return;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return;
        }

        auto pos = rt->GetScreenPosition();
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;

        // Draw background
        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            backgroundColor);

        // Calculate normalized value
        float normalizedValue = (maxValue - minValue) > 0.0f ? 
            (value - minValue) / (maxValue - minValue) : 0.0f;

        // Draw fill based on direction
        glm::vec2 fillPos = pos;
        glm::vec2 fillSize = scaledSize;

        switch (direction) {
            case ESliderDirection::LEFT_TO_RIGHT:
                fillSize.x *= normalizedValue;
                break;
            case ESliderDirection::RIGHT_TO_LEFT:
                fillPos.x += scaledSize.x * (1.0f - normalizedValue);
                fillSize.x *= normalizedValue;
                break;
            case ESliderDirection::BOTTOM_TO_TOP:
                fillSize.y *= normalizedValue;
                break;
            case ESliderDirection::TOP_TO_BOTTOM:
                fillPos.y += scaledSize.y * (1.0f - normalizedValue);
                fillSize.y *= normalizedValue;
                break;
        }

        if (normalizedValue > 0.0f) {
            pCanvas->DrawTexture2D(
                glm::vec3(fillPos, 0.01f),
                glm::vec3(fillPos.x + fillSize.x, fillPos.y + fillSize.y, 0.01f),
                glm::vec2(0.0f),
                glm::vec2(1.0f),
                nullptr,
                fillColor);
        }

        // Draw handle
        glm::vec2 handlePos;
        glm::vec2 scaledHandleSize = handleSize * uiScale;
        
        switch (direction) {
            case ESliderDirection::LEFT_TO_RIGHT:
                handlePos.x = pos.x + scaledSize.x * normalizedValue - scaledHandleSize.x * 0.5f;
                handlePos.y = pos.y + (scaledSize.y - scaledHandleSize.y) * 0.5f;
                break;
            case ESliderDirection::RIGHT_TO_LEFT:
                handlePos.x = pos.x + scaledSize.x * (1.0f - normalizedValue) - scaledHandleSize.x * 0.5f;
                handlePos.y = pos.y + (scaledSize.y - scaledHandleSize.y) * 0.5f;
                break;
            case ESliderDirection::BOTTOM_TO_TOP:
                handlePos.x = pos.x + (scaledSize.x - scaledHandleSize.x) * 0.5f;
                handlePos.y = pos.y + scaledSize.y * normalizedValue - scaledHandleSize.y * 0.5f;
                break;
            case ESliderDirection::TOP_TO_BOTTOM:
                handlePos.x = pos.x + (scaledSize.x - scaledHandleSize.x) * 0.5f;
                handlePos.y = pos.y + scaledSize.y * (1.0f - normalizedValue) - scaledHandleSize.y * 0.5f;
                break;
        }

        pCanvas->DrawTexture2D(
            glm::vec3(handlePos, 0.02f),
            glm::vec3(handlePos.x + scaledHandleSize.x, handlePos.y + scaledHandleSize.y, 0.02f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            handleColor);
    }

    void SliderWidgetComponent::LoadProperties(const nlohmann::json& json) {
        value = json.value("value", 0.0f);
        minValue = json.value("minValue", 0.0f);
        maxValue = json.value("maxValue", 1.0f);
        wholeNumbers = json.value("wholeNumbers", false);

        if (json.contains("backgroundColor")) {
            const auto& col = json["backgroundColor"];
            backgroundColor = {col.value("r", 0.3f), col.value("g", 0.3f), col.value("b", 0.3f), col.value("a", 1.0f)};
        }

        if (json.contains("fillColor")) {
            const auto& col = json["fillColor"];
            fillColor = {col.value("r", 0.2f), col.value("g", 0.6f), col.value("b", 1.0f), col.value("a", 1.0f)};
        }

        if (json.contains("handleColor")) {
            const auto& col = json["handleColor"];
            handleColor = {col.value("r", 1.0f), col.value("g", 1.0f), col.value("b", 1.0f), col.value("a", 1.0f)};
        }
    }

    bool SliderWidgetComponent::HitTest(const glm::vec2& point) const {
        if (!interactable) {
            return false;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return false;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto p1 = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        auto p2 = p1 + scaledSize;

        return (point.x >= p1.x) && (point.x <= p2.x) && (point.y >= p1.y) && (point.y <= p2.y);
    }

    void SliderWidgetComponent::OnPointerDown() {
        if (!interactable) {
            return;
        }
        
        isDragging = true;
        
        auto& input = Engine::GetInstance().GetInputManager();
        glm::vec2 mousePos = input.GetMousePosition();
        
        auto& renderer = Engine::GetInstance().GetSceneRenderer();
        const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
        mousePos.y = viewport.height - mousePos.y;

        float newValue = CalculateValueFromPosition(mousePos);
        SetValue(newValue);
    }

    void SliderWidgetComponent::OnPointerUp() {
        isDragging = false;
    }

    float SliderWidgetComponent::CalculateValueFromPosition(const glm::vec2& mousePos) const {
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return value;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto pos = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;

        float normalizedValue = 0.0f;

        switch (direction) {
            case ESliderDirection::LEFT_TO_RIGHT:
                normalizedValue = (mousePos.x - pos.x) / scaledSize.x;
                break;
            case ESliderDirection::RIGHT_TO_LEFT:
                normalizedValue = 1.0f - (mousePos.x - pos.x) / scaledSize.x;
                break;
            case ESliderDirection::BOTTOM_TO_TOP:
                normalizedValue = (mousePos.y - pos.y) / scaledSize.y;
                break;
            case ESliderDirection::TOP_TO_BOTTOM:
                normalizedValue = 1.0f - (mousePos.y - pos.y) / scaledSize.y;
                break;
        }

        normalizedValue = glm::clamp(normalizedValue, 0.0f, 1.0f);
        return minValue + normalizedValue * (maxValue - minValue);
    }

} // namespace golias
