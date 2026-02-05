#pragma once
#include "scene/ui/ui_base.h"
#include <functional>
#include <glm/vec4.hpp>

namespace golias {

    /// @brief Toggle transition type
    enum class EToggleTransition {
        NONE,
        FADE
    };

    using ToggleCallback = std::function<void(bool)>;

    /// @brief Toggle (checkbox) component
    class ToggleWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(ToggleWidgetComponent, WidgetComponent)
    public:
        void SetIsOn(bool on);
        bool IsOn() const;

        void SetInteractable(bool interactable);
        bool IsInteractable() const;

        void SetTransition(EToggleTransition transition);
        EToggleTransition GetTransition() const;

        void SetBackgroundColor(const glm::vec4& color);
        glm::vec4 GetBackgroundColor() const;

        void SetCheckmarkColor(const glm::vec4& color);
        glm::vec4 GetCheckmarkColor() const;

        void SetHoverColor(const glm::vec4& color);
        glm::vec4 GetHoverColor() const;

        void SetCheckmarkPadding(float padding);
        float GetCheckmarkPadding() const;

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;
        void OnPointerEnter() override;
        void OnPointerExit() override;
        void OnClick() override;

        /// @brief Called when toggle value changes
        ToggleCallback OnValueChanged;

    private:
        bool isOn = false;
        bool interactable = true;
        bool isHovered = false;
        
        EToggleTransition transition = EToggleTransition::NONE;
        
        glm::vec4 backgroundColor = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        glm::vec4 checkmarkColor = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
        glm::vec4 hoverColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        
        float checkmarkPadding = 4.0f;
        float fadeAlpha = 0.0f;
    };

} // namespace golias
