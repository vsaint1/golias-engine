#include "scene/components/widget/panel_component.h"

#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    bool PanelComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        if (properties.contains("color")) {
            const Json& colorObj = properties["color"];
            const float r        = colorObj.value("r", 1.0f);
            const float g        = colorObj.value("g", 1.0f);
            const float b        = colorObj.value("b", 1.0f);
            const float a        = colorObj.value("a", 1.0f);

            glm::vec4 color = glm::vec4(r, g, b, a);

            SetColor(color);
        }

        // if (properties.contains("border_color")) {
        //     const Json& c = properties["border_color"];

        //     const float r = c["r"].get<float>();
        //     const float g = c["g"].get<float>();
        //     const float b = c["b"].get<float>();
        //     const float a = c["a"].get<float>();

        //     mBorderColor = glm::vec4(r, g, b, a);
        // }

        // if (properties.contains("border_thickness")) {
        //     mBorderThickness = properties.value("border_thickness", 1.0f);
        // }

        if (properties.contains("pass_through")) {
            mPassThrough = properties.value("pass_through", true);
        }

        return true;
    }


    void PanelComponent::Start() {
    }

    void PanelComponent::Update(float deltaTime) {
    }

    void PanelComponent::Render(CanvasComponent* canvas) {
        if (!canvas) {
            return;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();

        if (!rectTransform) {
            return;
        }

        glm::vec2 pos = rectTransform->GetScreenPosition();
        pos -= rectTransform->GetPivot() * rectTransform->GetSize();

        canvas->DrawQuad(pos, pos + rectTransform->GetSize(), mColor);
    }

    bool PanelComponent::HitTest(const glm::vec2& point) {

        if (mPassThrough) {
            return false;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return false;
        }


        glm::vec2 pos = rectTransform->GetScreenPosition();
        glm::vec2 p1  = pos - rectTransform->GetPivot() * rectTransform->GetSize();
        glm::vec2 p2  = p1 + rectTransform->GetSize();


        return (point.x >= p1.x && point.x <= p2.x && point.y >= p1.y && point.y <= p2.y);
    }

    glm::vec4 PanelComponent::GetColor() const {
        return mColor;
    }

    void PanelComponent::SetColor(const glm::vec4& color) {
        mColor = color;
    }

} // namespace golias
