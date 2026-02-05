#include "scene/ui/layout_group_component.h"

#include "core/engine.h"
#include "scene/game_object.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    // ============== LayoutGroupComponent ==============

    void LayoutGroupComponent::SetPadding(const glm::vec4& pad) {
        padding = pad;
    }

    glm::vec4 LayoutGroupComponent::GetPadding() const {
        return padding;
    }

    void LayoutGroupComponent::SetSpacing(float sp) {
        spacing = sp;
    }

    float LayoutGroupComponent::GetSpacing() const {
        return spacing;
    }

    void LayoutGroupComponent::SetChildAlignment(ETextAnchor alignment) {
        childAlignment = alignment;
    }

    ETextAnchor LayoutGroupComponent::GetChildAlignment() const {
        return childAlignment;
    }

    void LayoutGroupComponent::SetControlChildWidth(bool control) {
        controlChildWidth = control;
    }

    bool LayoutGroupComponent::GetControlChildWidth() const {
        return controlChildWidth;
    }

    void LayoutGroupComponent::SetControlChildHeight(bool control) {
        controlChildHeight = control;
    }

    bool LayoutGroupComponent::GetControlChildHeight() const {
        return controlChildHeight;
    }

    void LayoutGroupComponent::SetChildForceExpandWidth(bool expand) {
        childForceExpandWidth = expand;
    }

    bool LayoutGroupComponent::GetChildForceExpandWidth() const {
        return childForceExpandWidth;
    }

    void LayoutGroupComponent::SetChildForceExpandHeight(bool expand) {
        childForceExpandHeight = expand;
    }

    bool LayoutGroupComponent::GetChildForceExpandHeight() const {
        return childForceExpandHeight;
    }

    void LayoutGroupComponent::Start() {
        CalculateLayout();
    }

    void LayoutGroupComponent::Update(float deltaTime) {
        CalculateLayout();
    }

    void LayoutGroupComponent::LoadProperties(const nlohmann::json& json) {
        if (json.contains("padding")) {
            const auto& pad = json["padding"];
            padding = {pad.value("left", 0.0f), pad.value("bottom", 0.0f), pad.value("right", 0.0f), pad.value("top", 0.0f)};
        }
        
        spacing = json.value("spacing", 0.0f);
        controlChildWidth = json.value("controlChildWidth", true);
        controlChildHeight = json.value("controlChildHeight", true);
        childForceExpandWidth = json.value("childForceExpandWidth", false);
        childForceExpandHeight = json.value("childForceExpandHeight", false);
    }

    glm::vec2 LayoutGroupComponent::GetAlignmentOffset(const glm::vec2& totalSize, const glm::vec2& containerSize) const {
        glm::vec2 offset(0.0f);
        glm::vec2 availableSize = containerSize - glm::vec2(padding.x + padding.z, padding.y + padding.w);
        
        switch (childAlignment) {
            case ETextAnchor::UPPER_LEFT:
            case ETextAnchor::MIDDLE_LEFT:
            case ETextAnchor::LOWER_LEFT:
                offset.x = 0.0f;
                break;
            case ETextAnchor::UPPER_CENTER:
            case ETextAnchor::MIDDLE_CENTER:
            case ETextAnchor::LOWER_CENTER:
                offset.x = (availableSize.x - totalSize.x) * 0.5f;
                break;
            case ETextAnchor::UPPER_RIGHT:
            case ETextAnchor::MIDDLE_RIGHT:
            case ETextAnchor::LOWER_RIGHT:
                offset.x = availableSize.x - totalSize.x;
                break;
        }
        
        switch (childAlignment) {
            case ETextAnchor::UPPER_LEFT:
            case ETextAnchor::UPPER_CENTER:
            case ETextAnchor::UPPER_RIGHT:
                offset.y = availableSize.y - totalSize.y;
                break;
            case ETextAnchor::MIDDLE_LEFT:
            case ETextAnchor::MIDDLE_CENTER:
            case ETextAnchor::MIDDLE_RIGHT:
                offset.y = (availableSize.y - totalSize.y) * 0.5f;
                break;
            case ETextAnchor::LOWER_LEFT:
            case ETextAnchor::LOWER_CENTER:
            case ETextAnchor::LOWER_RIGHT:
                offset.y = 0.0f;
                break;
        }
        
        return offset;
    }

    // ============== HorizontalLayoutGroupComponent ==============

    void HorizontalLayoutGroupComponent::SetReverseArrangement(bool reverse) {
        reverseArrangement = reverse;
    }

    bool HorizontalLayoutGroupComponent::GetReverseArrangement() const {
        return reverseArrangement;
    }

    void HorizontalLayoutGroupComponent::Start() {
        LayoutGroupComponent::Start();
    }

    void HorizontalLayoutGroupComponent::Update(float deltaTime) {
        LayoutGroupComponent::Update(deltaTime);
    }

    void HorizontalLayoutGroupComponent::LoadProperties(const nlohmann::json& json) {
        LayoutGroupComponent::LoadProperties(json);
        reverseArrangement = json.value("reverseArrangement", false);
    }

    void HorizontalLayoutGroupComponent::CalculateLayout() {
        auto owner = GetOwner();
        if (!owner) return;
        
        auto parentRt = owner->GetComponent<RectTransformComponent>();
        if (!parentRt) return;
        
        const auto& children = owner->GetChildren();
        if (children.empty()) return;
        
        std::vector<RectTransformComponent*> childRects;
        float totalWidth = 0.0f;
        float maxHeight = 0.0f;
        
        for (const auto& child : children) {
            auto childRt = child->GetComponent<RectTransformComponent>();
            if (childRt) {
                childRects.push_back(childRt);
                totalWidth += childRt->GetSize().x;
                maxHeight = std::max(maxHeight, childRt->GetSize().y);
            }
        }
        
        if (childRects.empty()) return;
        
        totalWidth += spacing * (childRects.size() - 1);
        
        glm::vec2 containerSize = parentRt->GetSize();
        glm::vec2 availableSize = containerSize - glm::vec2(padding.x + padding.z, padding.y + padding.w);
        
        // Calculate child width if controlling or expanding
        float childWidth = 0.0f;
        if (childForceExpandWidth) {
            childWidth = (availableSize.x - spacing * (childRects.size() - 1)) / childRects.size();
            totalWidth = availableSize.x;
        }
        
        // Get alignment offset
        glm::vec2 alignOffset = GetAlignmentOffset(glm::vec2(totalWidth, maxHeight), containerSize);
        
        // Position children
        float currentX = padding.x + alignOffset.x;
        
        if (reverseArrangement) {
            currentX = containerSize.x - padding.z - alignOffset.x;
        }
        
        for (size_t i = 0; i < childRects.size(); ++i) {
            size_t idx = reverseArrangement ? (childRects.size() - 1 - i) : i;
            auto childRt = childRects[idx];
            auto childObj = childRt->GetOwner();
            
            glm::vec2 childSize = childRt->GetSize();
            
            if (childForceExpandWidth) {
                childSize.x = childWidth;
            }
            if (controlChildHeight) {
                childSize.y = availableSize.y;
            }
            if (controlChildWidth) {
                childRt->SetSize(childSize);
            }
            
            float posY = padding.y + alignOffset.y;
            
            if (reverseArrangement) {
                currentX -= childSize.x;
                childObj->SetPosition({currentX, posY, 0.0f});
                currentX -= spacing;
            } else {
                childObj->SetPosition({currentX, posY, 0.0f});
                currentX += childSize.x + spacing;
            }
            
            childRt->SetAnchor({0.0f, 0.0f});
            childRt->SetPivot({0.0f, 0.0f});
        }
    }

    // ============== VerticalLayoutGroupComponent ==============

    void VerticalLayoutGroupComponent::SetReverseArrangement(bool reverse) {
        reverseArrangement = reverse;
    }

    bool VerticalLayoutGroupComponent::GetReverseArrangement() const {
        return reverseArrangement;
    }

    void VerticalLayoutGroupComponent::Start() {
        LayoutGroupComponent::Start();
    }

    void VerticalLayoutGroupComponent::Update(float deltaTime) {
        LayoutGroupComponent::Update(deltaTime);
    }

    void VerticalLayoutGroupComponent::LoadProperties(const nlohmann::json& json) {
        LayoutGroupComponent::LoadProperties(json);
        reverseArrangement = json.value("reverseArrangement", false);
    }

    void VerticalLayoutGroupComponent::CalculateLayout() {
        auto owner = GetOwner();
        if (!owner) return;
        
        auto parentRt = owner->GetComponent<RectTransformComponent>();
        if (!parentRt) return;
        
        const auto& children = owner->GetChildren();
        if (children.empty()) return;
        
        std::vector<RectTransformComponent*> childRects;
        float totalHeight = 0.0f;
        float maxWidth = 0.0f;
        
        for (const auto& child : children) {
            auto childRt = child->GetComponent<RectTransformComponent>();
            if (childRt) {
                childRects.push_back(childRt);
                totalHeight += childRt->GetSize().y;
                maxWidth = std::max(maxWidth, childRt->GetSize().x);
            }
        }
        
        if (childRects.empty()) return;
        
        totalHeight += spacing * (childRects.size() - 1);
        
        glm::vec2 containerSize = parentRt->GetSize();
        glm::vec2 availableSize = containerSize - glm::vec2(padding.x + padding.z, padding.y + padding.w);
        
        // Calculate child height if expanding
        float childHeight = 0.0f;
        if (childForceExpandHeight) {
            childHeight = (availableSize.y - spacing * (childRects.size() - 1)) / childRects.size();
            totalHeight = availableSize.y;
        }
        
        // Get alignment offset
        glm::vec2 alignOffset = GetAlignmentOffset(glm::vec2(maxWidth, totalHeight), containerSize);
        
        // Position children from top to bottom
        float currentY = containerSize.y - padding.w - alignOffset.y;
        
        if (reverseArrangement) {
            currentY = padding.y + alignOffset.y;
        }
        
        for (size_t i = 0; i < childRects.size(); ++i) {
            size_t idx = reverseArrangement ? (childRects.size() - 1 - i) : i;
            auto childRt = childRects[idx];
            auto childObj = childRt->GetOwner();
            
            glm::vec2 childSize = childRt->GetSize();
            
            if (childForceExpandHeight) {
                childSize.y = childHeight;
            }
            if (controlChildWidth) {
                childSize.x = availableSize.x;
            }
            if (controlChildHeight) {
                childRt->SetSize(childSize);
            }
            
            float posX = padding.x + alignOffset.x;
            
            if (reverseArrangement) {
                childObj->SetPosition({posX, currentY, 0.0f});
                currentY += childSize.y + spacing;
            } else {
                currentY -= childSize.y;
                childObj->SetPosition({posX, currentY, 0.0f});
                currentY -= spacing;
            }
            
            childRt->SetAnchor({0.0f, 0.0f});
            childRt->SetPivot({0.0f, 0.0f});
        }
    }

    // ============== GridLayoutGroupComponent ==============

    void GridLayoutGroupComponent::SetCellSize(const glm::vec2& size) {
        cellSize = size;
    }

    glm::vec2 GridLayoutGroupComponent::GetCellSize() const {
        return cellSize;
    }

    void GridLayoutGroupComponent::SetSpacingXY(const glm::vec2& sp) {
        spacingXY = sp;
    }

    glm::vec2 GridLayoutGroupComponent::GetSpacingXY() const {
        return spacingXY;
    }

    void GridLayoutGroupComponent::SetStartCorner(ELayoutCorner corner) {
        startCorner = corner;
    }

    ELayoutCorner GridLayoutGroupComponent::GetStartCorner() const {
        return startCorner;
    }

    void GridLayoutGroupComponent::SetStartAxis(ELayoutAxis axis) {
        startAxis = axis;
    }

    ELayoutAxis GridLayoutGroupComponent::GetStartAxis() const {
        return startAxis;
    }

    void GridLayoutGroupComponent::SetConstraint(EGridConstraint c) {
        constraint = c;
    }

    EGridConstraint GridLayoutGroupComponent::GetConstraint() const {
        return constraint;
    }

    void GridLayoutGroupComponent::SetConstraintCount(int count) {
        constraintCount = std::max(1, count);
    }

    int GridLayoutGroupComponent::GetConstraintCount() const {
        return constraintCount;
    }

    void GridLayoutGroupComponent::Start() {
        LayoutGroupComponent::Start();
    }

    void GridLayoutGroupComponent::Update(float deltaTime) {
        LayoutGroupComponent::Update(deltaTime);
    }

    void GridLayoutGroupComponent::LoadProperties(const nlohmann::json& json) {
        LayoutGroupComponent::LoadProperties(json);
        
        if (json.contains("cellSize")) {
            const auto& size = json["cellSize"];
            cellSize = {size.value("x", 100.0f), size.value("y", 100.0f)};
        }
        
        if (json.contains("spacingXY")) {
            const auto& sp = json["spacingXY"];
            spacingXY = {sp.value("x", 0.0f), sp.value("y", 0.0f)};
        }
        
        constraintCount = json.value("constraintCount", 2);
    }

    void GridLayoutGroupComponent::CalculateLayout() {
        auto owner = GetOwner();
        if (!owner) return;
        
        auto parentRt = owner->GetComponent<RectTransformComponent>();
        if (!parentRt) return;
        
        const auto& children = owner->GetChildren();
        if (children.empty()) return;
        
        std::vector<RectTransformComponent*> childRects;
        for (const auto& child : children) {
            auto childRt = child->GetComponent<RectTransformComponent>();
            if (childRt) {
                childRects.push_back(childRt);
            }
        }
        
        if (childRects.empty()) return;
        
        glm::vec2 containerSize = parentRt->GetSize();
        glm::vec2 availableSize = containerSize - glm::vec2(padding.x + padding.z, padding.y + padding.w);
        
        int columns = 1;
        int rows = 1;
        
        switch (constraint) {
            case EGridConstraint::FIXED_COLUMN_COUNT:
                columns = constraintCount;
                rows = static_cast<int>(std::ceil(static_cast<float>(childRects.size()) / columns));
                break;
            case EGridConstraint::FIXED_ROW_COUNT:
                rows = constraintCount;
                columns = static_cast<int>(std::ceil(static_cast<float>(childRects.size()) / rows));
                break;
            case EGridConstraint::FLEXIBLE:
            default:
                columns = std::max(1, static_cast<int>((availableSize.x + spacingXY.x) / (cellSize.x + spacingXY.x)));
                rows = static_cast<int>(std::ceil(static_cast<float>(childRects.size()) / columns));
                break;
        }
        
        // Position children
        for (size_t i = 0; i < childRects.size(); ++i) {
            auto childRt = childRects[i];
            auto childObj = childRt->GetOwner();
            
            int col, row;
            if (startAxis == ELayoutAxis::HORIZONTAL) {
                col = i % columns;
                row = i / columns;
            } else {
                row = i % rows;
                col = i / rows;
            }
            
            float posX, posY;
            
            switch (startCorner) {
                case ELayoutCorner::UPPER_LEFT:
                    posX = padding.x + col * (cellSize.x + spacingXY.x);
                    posY = containerSize.y - padding.w - cellSize.y - row * (cellSize.y + spacingXY.y);
                    break;
                case ELayoutCorner::UPPER_RIGHT:
                    posX = containerSize.x - padding.z - cellSize.x - col * (cellSize.x + spacingXY.x);
                    posY = containerSize.y - padding.w - cellSize.y - row * (cellSize.y + spacingXY.y);
                    break;
                case ELayoutCorner::LOWER_LEFT:
                    posX = padding.x + col * (cellSize.x + spacingXY.x);
                    posY = padding.y + row * (cellSize.y + spacingXY.y);
                    break;
                case ELayoutCorner::LOWER_RIGHT:
                default:
                    posX = containerSize.x - padding.z - cellSize.x - col * (cellSize.x + spacingXY.x);
                    posY = padding.y + row * (cellSize.y + spacingXY.y);
                    break;
            }
            
            childObj->SetPosition({posX, posY, 0.0f});
            childRt->SetSize(cellSize);
            childRt->SetAnchor({0.0f, 0.0f});
            childRt->SetPivot({0.0f, 0.0f});
        }
    }

} // namespace golias
