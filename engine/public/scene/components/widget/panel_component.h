#pragma once

#include "widget_component.h"


namespace golias {

    class PanelComponent : public WidgetComponent {
        COMPONENT_DERIVED(PanelComponent, WidgetComponent)
    public:
        bool LoadProperties(const Json& properties);

        void Start() override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas);

        bool HitTest(const glm::vec2& point);

        glm::vec4 GetColor() const;
        void SetColor(const glm::vec4& color);

    private:
        glm::vec4 mColor = glm::vec4(1.0f);

        glm::vec4 mBorderColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float mBorderThickness = 1.0f;

        bool mPassThrough = true;
    };
} // namespace golias
