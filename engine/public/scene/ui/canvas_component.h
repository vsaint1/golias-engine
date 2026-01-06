#pragma once
#include "scene/ui/ui_base.h"

namespace golias {

    class CanvasComponent : public Component {
        COMPONENT(CanvasComponent)
    public:
        void Update(float deltaTime) override;

        void Draw(WidgetComponent* pWidget);
   
    };

} // namespace golias
