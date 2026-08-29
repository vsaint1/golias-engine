#pragma once

#include "scene/components/component.h"

namespace golias {

    class CanvasComponent;

    class WidgetComponent : public Component {
        COMPONENT(WidgetComponent)
    public:
        WidgetComponent()  = default;
        ~WidgetComponent() = default;

        void Update(float deltaTime) override;

        virtual void Render(CanvasComponent* canvas);

    protected:
        glm::vec2 mPivot = glm::vec2(0.5f);
    };
} // namespace golias
