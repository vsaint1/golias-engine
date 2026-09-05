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

        virtual bool HitTest(const glm::vec2& point);

        virtual glm::vec2 GetDesiredSize() const;

        virtual void OnPointerEnter();
        virtual void OnPointerExit();

        virtual void OnPointerUp();
        virtual void OnPointerDown();

        virtual void OnClick();

        virtual bool IsTopmost() const;

  
    };
} // namespace golias
