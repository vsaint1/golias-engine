#include "scene/ui/progressbar_component.h"

#include "core/engine.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void ProgressBarWidgetComponent::SetValue(float val) {
        value = glm::clamp(val, minValue, maxValue);
    }

    float ProgressBarWidgetComponent::GetValue() const {
        return value;
    }

    void ProgressBarWidgetComponent::SetMinValue(float min) {
        minValue = min;
        SetValue(value);
    }

    float ProgressBarWidgetComponent::GetMinValue() const {
        return minValue;
    }

    void ProgressBarWidgetComponent::SetMaxValue(float max) {
        maxValue = max;
        SetValue(value);
    }

    float ProgressBarWidgetComponent::GetMaxValue() const {
        return maxValue;
    }

    void ProgressBarWidgetComponent::SetBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }

    glm::vec4 ProgressBarWidgetComponent::GetBackgroundColor() const {
        return backgroundColor;
    }

    void ProgressBarWidgetComponent::SetFillColor(const glm::vec4& color) {
        fillColor = color;
    }

    glm::vec4 ProgressBarWidgetComponent::GetFillColor() const {
        return fillColor;
    }

    void ProgressBarWidgetComponent::SetFillDirection(int direction) {
        fillDirection = glm::clamp(direction, 0, 3);
    }

    int ProgressBarWidgetComponent::GetFillDirection() const {
        return fillDirection;
    }

    void ProgressBarWidgetComponent::Start() {
    }

    void ProgressBarWidgetComponent::Update(float deltaTime) {
    }

    void ProgressBarWidgetComponent::Draw(CanvasComponent* pCanvas) {
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

        // Calculate fill amount
        float normalizedValue = (maxValue - minValue) > 0.0f ?
            (value - minValue) / (maxValue - minValue) : 0.0f;

        if (normalizedValue <= 0.0f) {
            return;
        }

        glm::vec2 fillPos = pos;
        glm::vec2 fillSize = scaledSize;

        switch (fillDirection) {
            case 0: // Left to right
                fillSize.x *= normalizedValue;
                break;
            case 1: // Right to left
                fillPos.x += scaledSize.x * (1.0f - normalizedValue);
                fillSize.x *= normalizedValue;
                break;
            case 2: // Bottom to top
                fillSize.y *= normalizedValue;
                break;
            case 3: // Top to bottom
                fillPos.y += scaledSize.y * (1.0f - normalizedValue);
                fillSize.y *= normalizedValue;
                break;
        }

        pCanvas->DrawTexture2D(
            glm::vec3(fillPos, 0.01f),
            glm::vec3(fillPos.x + fillSize.x, fillPos.y + fillSize.y, 0.01f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            fillColor);
    }

    void ProgressBarWidgetComponent::LoadProperties(const nlohmann::json& json) {
        value = json.value("value", 0.5f);
        minValue = json.value("minValue", 0.0f);
        maxValue = json.value("maxValue", 1.0f);
        fillDirection = json.value("fillDirection", 0);

        if (json.contains("backgroundColor")) {
            const auto& col = json["backgroundColor"];
            backgroundColor = {col.value("r", 0.2f), col.value("g", 0.2f), col.value("b", 0.2f), col.value("a", 1.0f)};
        }

        if (json.contains("fillColor")) {
            const auto& col = json["fillColor"];
            fillColor = {col.value("r", 0.2f), col.value("g", 0.7f), col.value("b", 0.2f), col.value("a", 1.0f)};
        }
    }

} // namespace golias
