#pragma once
#include "scene/ui/ui_base.h"

namespace golias {

    class ButtonWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(ButtonWidgetComponent, WidgetComponent)
    public:
        void SetRect(const glm::vec2& size);
        glm::vec2 GetRect() const;

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
    private:
        glm::vec2 rect;
        glm::vec4 color = glm::vec4(1.0f);
    };
} // namespace golias
