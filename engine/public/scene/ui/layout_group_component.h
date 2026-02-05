#pragma once
#include "scene/ui/ui_base.h"
#include <glm/vec4.hpp>

namespace golias {

    /// @brief Text alignment for layout groups
    enum class ETextAnchor {
        UPPER_LEFT,
        UPPER_CENTER,
        UPPER_RIGHT,
        MIDDLE_LEFT,
        MIDDLE_CENTER,
        MIDDLE_RIGHT,
        LOWER_LEFT,
        LOWER_CENTER,
        LOWER_RIGHT
    };

    /// @brief Corner for layout start position
    enum class ELayoutCorner {
        UPPER_LEFT,
        UPPER_RIGHT,
        LOWER_LEFT,
        LOWER_RIGHT
    };

    /// @brief Axis constraint for layout
    enum class ELayoutAxis {
        HORIZONTAL,
        VERTICAL
    };

    /// @brief Constraint for grid layout
    enum class EGridConstraint {
        FLEXIBLE,
        FIXED_COLUMN_COUNT,
        FIXED_ROW_COUNT
    };

    /// @brief Base class for layout group components
    class LayoutGroupComponent : public Component {
        COMPONENT(LayoutGroupComponent)
    public:
        void SetPadding(const glm::vec4& padding);
        glm::vec4 GetPadding() const;

        void SetSpacing(float spacing);
        float GetSpacing() const;

        void SetChildAlignment(ETextAnchor alignment);
        ETextAnchor GetChildAlignment() const;

        void SetControlChildWidth(bool control);
        bool GetControlChildWidth() const;

        void SetControlChildHeight(bool control);
        bool GetControlChildHeight() const;

        void SetChildForceExpandWidth(bool expand);
        bool GetChildForceExpandWidth() const;

        void SetChildForceExpandHeight(bool expand);
        bool GetChildForceExpandHeight() const;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

    protected:
        virtual void CalculateLayout() {}
        glm::vec2 GetAlignmentOffset(const glm::vec2& totalSize, const glm::vec2& containerSize) const;

        glm::vec4 padding = glm::vec4(0.0f); // left, bottom, right, top
        float spacing = 0.0f;
        ETextAnchor childAlignment = ETextAnchor::UPPER_LEFT;
        
        bool controlChildWidth = true;
        bool controlChildHeight = true;
        bool childForceExpandWidth = false;
        bool childForceExpandHeight = false;
    };

    /// @brief Horizontal layout group
    class HorizontalLayoutGroupComponent : public LayoutGroupComponent {
        COMPONENT_DERIVED(HorizontalLayoutGroupComponent, LayoutGroupComponent)
    public:
        void SetReverseArrangement(bool reverse);
        bool GetReverseArrangement() const;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

    protected:
        void CalculateLayout() override;

    private:
        bool reverseArrangement = false;
    };

    /// @brief Vertical layout group 
    class VerticalLayoutGroupComponent : public LayoutGroupComponent {
        COMPONENT_DERIVED(VerticalLayoutGroupComponent, LayoutGroupComponent)
    public:
        void SetReverseArrangement(bool reverse);
        bool GetReverseArrangement() const;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

    protected:
        void CalculateLayout() override;

    private:
        bool reverseArrangement = false;
    };

    /// @brief Grid layout group
    class GridLayoutGroupComponent : public LayoutGroupComponent {
        COMPONENT_DERIVED(GridLayoutGroupComponent, LayoutGroupComponent)
    public:
        void SetCellSize(const glm::vec2& size);
        glm::vec2 GetCellSize() const;

        void SetSpacingXY(const glm::vec2& spacing);
        glm::vec2 GetSpacingXY() const;

        void SetStartCorner(ELayoutCorner corner);
        ELayoutCorner GetStartCorner() const;

        void SetStartAxis(ELayoutAxis axis);
        ELayoutAxis GetStartAxis() const;

        void SetConstraint(EGridConstraint constraint);
        EGridConstraint GetConstraint() const;

        void SetConstraintCount(int count);
        int GetConstraintCount() const;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

    protected:
        void CalculateLayout() override;

    private:
        glm::vec2 cellSize = glm::vec2(100.0f, 100.0f);
        glm::vec2 spacingXY = glm::vec2(0.0f);
        ELayoutCorner startCorner = ELayoutCorner::UPPER_LEFT;
        ELayoutAxis startAxis = ELayoutAxis::HORIZONTAL;
        EGridConstraint constraint = EGridConstraint::FLEXIBLE;
        int constraintCount = 2;
    };

} // namespace golias
