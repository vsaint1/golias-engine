#include "scene/components/widget/rect_transform_component.h"

#include "scene/game_object.h"

namespace golias {

    bool RectTransformComponent::LoadProperties(const Json& properties) {

        if (properties.contains("size")) {
            const Json& sizeJson = properties["size"];
            const float x        = sizeJson.value("x", 0.0f);
            const float y        = sizeJson.value("y", 0.0f);

            mSize = glm::vec2(x, y);
        }

        if (properties.contains("anchor")) {
            const Json& anchorPointJson = properties["anchor"];
            const float x               = anchorPointJson.value("x", 0.0f);
            const float y               = anchorPointJson.value("y", 0.0f);

            mAnchorPoint = glm::vec2(x, y);
        }

        if (properties.contains("pivot")) {
            const Json& pivotJson = properties["pivot"];
            const float x         = pivotJson.value("x", 0.5f);
            const float y         = pivotJson.value("y", 0.5f);

            mPivot = glm::vec2(x, y);
        }


        return true;
    }

    void RectTransformComponent::Update(float deltaTime) {
    }

    glm::vec2 RectTransformComponent::GetSize() const {
        return mSize;
    }

    void RectTransformComponent::SetSize(const glm::vec2& size) {
        mSize = size;
    }

    glm::vec2 RectTransformComponent::GetAnchorPoint() const {
        return mAnchorPoint;
    }

    void RectTransformComponent::SetAnchorPoint(const glm::vec2& anchorPoint) {
        mAnchorPoint = anchorPoint;
    }

    glm::vec2 RectTransformComponent::GetPivot() const {
        return mPivot;
    }

    void RectTransformComponent::SetPivot(const glm::vec2& pivot) {
        mPivot = pivot;
    }

    glm::vec2 RectTransformComponent::GetScreenPosition() const {
        GameObject* parent = GetOwner()->GetParent();

        if (!parent || !parent->GetComponent<RectTransformComponent>()) {
            return GetOwner()->GetPosition2D();
        }

        RectTransformComponent* parentRect = parent->GetComponent<RectTransformComponent>();
        const glm::vec2 parentTopLeft      = parentRect->GetScreenPosition() - parentRect->GetPivot() * parentRect->GetSize();

        return parentTopLeft + mAnchorPoint * parentRect->GetSize() + GetOwner()->GetPosition2D();
    }

} // namespace golias
