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

        glm::vec2 GetPivot() const;

        void SetPivot(const glm::vec2& value);

    protected:
        glm::vec2 pivot = glm::vec2(0.5f);
    };

}; // namespace golias
