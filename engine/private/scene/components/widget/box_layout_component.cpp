#include "scene/components/widget/box_layout_component.h"

#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {


    bool BoxLayoutComponent::LoadProperties(const Json& properties) {

        const String direction = properties.value("direction", "vertical");
        mIsVertical            = (direction != "horizontal");

        mSpacing = properties.value("spacing", 4.0f);

        if (properties.contains("padding") && properties["padding"].is_object()) {
            const Json& padding = properties["padding"];
            mPadding            = glm::vec4(
                padding.value("left", 0.0f), padding.value("top", 0.0f), padding.value("right", 0.0f), padding.value("bottom", 0.0f));
        }

        const String alignment = properties.value("alignment", "center");
        if (alignment == "start") {
            mAlignment = Alignment::Start;
        } else if (alignment == "end") {
            mAlignment = Alignment::End;
        } else if (alignment == "stretch") {
            mAlignment = Alignment::Stretch;
        } else {
            mAlignment = Alignment::Center;
        }

        return true;
    }

    void BoxLayoutComponent::Update(float deltaTime) {
    }

    void BoxLayoutComponent::Render(CanvasComponent* canvas) {
        Arrange();
    }

    bool BoxLayoutComponent::HitTest(const glm::vec2& point) {
        return false;
    }

    void BoxLayoutComponent::Arrange() {
        RectTransformComponent* rect = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rect) {
            return;
        }

        const glm::vec2 containerSize = rect->GetSize();
        const glm::vec2 topLeft       = rect->GetScreenPosition() - rect->GetPivot() * containerSize;

        const int mainAxis  = mIsVertical ? 1 : 0;
        const int crossAxis = mIsVertical ? 0 : 1;

        const float padMainStart  = mIsVertical ? mPadding.y : mPadding.x;
        const float padMainEnd    = mIsVertical ? mPadding.w : mPadding.z;
        const float padCrossStart = mIsVertical ? mPadding.x : mPadding.y;
        const float padCrossEnd   = mIsVertical ? mPadding.z : mPadding.w;

        const float totalCross = containerSize[crossAxis] - padCrossStart - padCrossEnd;

        float cursor = padMainStart;

        for (const auto& child : GetOwner()->GetChildren()) {

            WidgetComponent* widget = child->GetComponent<WidgetComponent>();
            if (!widget) {
                continue;
            }

            RectTransformComponent* childRect = child->GetComponent<RectTransformComponent>();
            if (!childRect) {
                continue;
            }

            glm::vec2 size = widget->GetDesiredSize();

            glm::vec2 offset = glm::vec2(0.0f);

            switch (mAlignment) {
            case Alignment::Start:
                offset[crossAxis] = padCrossStart;
                break;
            case Alignment::End:
                offset[crossAxis] = padCrossStart + (totalCross - size[crossAxis]);
                break;
            case Alignment::Stretch:
                size[crossAxis]   = totalCross;
                offset[crossAxis] = padCrossStart;
                break;
            case Alignment::Center:
            default:
                offset[crossAxis] = padCrossStart + (totalCross - size[crossAxis]) * 0.5f;
                break;
            }

            offset[mainAxis] = cursor;

            childRect->SetAnchorPoint(glm::vec2(0.0f));
            childRect->SetPivot(glm::vec2(0.0f));
            childRect->SetSize(size);
            child->SetPosition2D(offset);

            cursor += size[mainAxis] + mSpacing;
        }
    }

} // namespace golias
