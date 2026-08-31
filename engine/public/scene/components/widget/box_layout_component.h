#pragma once

#include "scene/components/widget/widget_component.h"

namespace golias {

    class BoxLayoutComponent : public WidgetComponent {
        COMPONENT_DERIVED(BoxLayoutComponent, WidgetComponent)
    public:
        BoxLayoutComponent()  = default;
        ~BoxLayoutComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        bool HitTest(const glm::vec2& point) override;

        void Arrange();

    private:
        enum class Alignment {
            Start   = 0,
            Center  = 1,
            End     = 2,
            Stretch = 3,
        };

        bool mIsVertical = true;

        float mSpacing = 4.0f;

        glm::vec4 mPadding = glm::vec4(0.0f); // (left, top, right, bottom)

        Alignment mAlignment = Alignment::Center;
    };

} // namespace golias
