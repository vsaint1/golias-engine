#pragma once
#include "scene/ui/ui_base.h"
#include <functional>


namespace golias {

    using Delegate = std::function<void()>;

    class ButtonWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(ButtonWidgetComponent, WidgetComponent)
    public:
 
        void SetColor(const glm::vec4& value);
        glm::vec4 GetColor() const;

        bool HitTest(const glm::vec2& point) const override;
        void OnPointerEnter() override;
        void OnPointerExit() override;
        void OnPointerDown() override;
        void OnPointerUp() override;
        void OnClick() override;

        void Draw(CanvasComponent* pCanvas) override;
        void Start() override;
        void Update(float deltaTime) override;

        Delegate OnButtonClick;

    private:
        
        glm::vec4 normalColor  = glm::vec4(1.0f);
        glm::vec4 hoverColor   = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        glm::vec4 pressedColor = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);

        glm::vec4 color = normalColor;
    };
} // namespace golias
