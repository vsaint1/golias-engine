#include "scene/ui/rect_transform_component.h"

#include "core/engine.h"
#include "scene/game_object.h"

namespace golias {
    RectTransformComponent::RectTransformComponent() {
    }

    glm::vec2 RectTransformComponent::GetSize() const {
        // If stretch mode is enabled, dynamically match parent's size
        if (mStretchToParent) {
            auto parent = GetOwner()->GetParent();
            if (parent) {
                if (auto parentRect = parent->GetComponent<RectTransformComponent>()) {
                    return parentRect->GetSize();
                }
            }
        }
        return mSize;
    }

    void RectTransformComponent::SetSize(const glm::vec2& size) {
        mSize = size;
    }

    glm::vec2 RectTransformComponent::GetAnchor() const {
        return mAnchor;
    }

    void RectTransformComponent::SetAnchor(const glm::vec2& anchor) {
        mAnchor = anchor;
    }

    glm::vec2 RectTransformComponent::GetPivot() const {
        return mPivot;
    }

    void RectTransformComponent::SetPivot(const glm::vec2& pivot) {
        mPivot = pivot;
    }

    void RectTransformComponent::SetStretchToParent(bool stretch) {
        mStretchToParent = stretch;
    }

    bool RectTransformComponent::GetStretchToParent() const {
        return mStretchToParent;
    }

    void RectTransformComponent::Start() {
    }

    void RectTransformComponent::Update(float deltaTime) {
        // If stretch mode is enabled, match parent's size
        if (mStretchToParent) {
            auto parent = GetOwner()->GetParent();
            if (parent) {
                if (auto parentRect = parent->GetComponent<RectTransformComponent>()) {
                    mSize = parentRect->GetSize();
                }
            }
        }
    }

    void RectTransformComponent::LoadProperties(const nlohmann::json& json) {
        if (json.contains("size")) {
            mSize = glm::vec2(json["size"]["x"], json["size"]["y"]);
        }
        if (json.contains("anchor")) {
            mAnchor = glm::vec2(json["anchor"]["x"], json["anchor"]["y"]);
        }
        if (json.contains("pivot")) {
            mPivot = glm::vec2(json["pivot"]["x"], json["pivot"]["y"]);
        }
        if (json.contains("stretchToParent")) {
            mStretchToParent = json["stretchToParent"];
        }
    }


    glm::vec2 RectTransformComponent::GetScreenPosition() const {

        auto parent = GetOwner()->GetParent();
        if (!parent || !parent->GetComponent<RectTransformComponent>()) {
            return GetOwner()->GetPosition2D();
        }

        auto parentRect     = parent->GetComponent<RectTransformComponent>();
        glm::vec2 parentPos = parentRect->GetScreenPosition();
        glm::vec2 parentSize = parentRect->GetSize();
        
        // Calculate anchor position within parent (anchor 0,0 = bottom-left, 1,1 = top-right)
        // Anchors work on actual viewport size, not scaled
        glm::vec2 anchorPos = parentPos + mAnchor * parentSize;
        
        // Get UI scale from renderer
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        
        // Add local offset (scaled) and adjust for pivot (scaled)
        // Use GetSize() instead of mSize to handle stretchToParent correctly
        glm::vec2 localPos = GetOwner()->GetPosition2D() * uiScale;
        glm::vec2 pivotOffset = mPivot * GetSize() * uiScale;
        
        return anchorPos + localPos - pivotOffset;
    }
} // namespace golias
