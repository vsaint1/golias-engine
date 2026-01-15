#pragma once
#include "scene/component.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace golias {

    class CanvasComponent;

    class WidgetComponent : public Component {
        COMPONENT(WidgetComponent)

    public:
        virtual void Draw(CanvasComponent* pCanvas);

        void SetPivot(const glm::vec2& value);
        glm::vec2 GetPivot() const;

        virtual bool HitTest(const glm::vec2& point) const;
        
        virtual void OnPointerEnter();
        virtual void OnPointerExit();
        virtual void OnPointerDown();
        virtual void OnPointerUp();
        virtual void OnClick();

    protected:
        glm::vec2 pivot = glm::vec2(0.5f);
    };

}; // namespace golias
