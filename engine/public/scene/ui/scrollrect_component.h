#pragma once
#include "scene/ui/ui_base.h"
#include <functional>
#include <glm/vec4.hpp>

namespace golias {

    /// @brief Scroll movement type
    enum class EScrollMovementType {
        UNRESTRICTED,
        ELASTIC,
        CLAMPED
    };

    /// @brief Scrollbar visibility mode
    enum class EScrollbarVisibility {
        PERMANENT,
        AUTO_HIDE,
        AUTO_HIDE_AND_EXPAND
    };

    using ScrollCallback = std::function<void(const glm::vec2&)>;

    /// @brief ScrollRect component for scrollable content
    class ScrollRectWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(ScrollRectWidgetComponent, WidgetComponent)
    public:
        void SetHorizontalScrollEnabled(bool enabled);
        bool IsHorizontalScrollEnabled() const;

        void SetVerticalScrollEnabled(bool enabled);
        bool IsVerticalScrollEnabled() const;

        void SetMovementType(EScrollMovementType type);
        EScrollMovementType GetMovementType() const;

        void SetElasticity(float elasticity);
        float GetElasticity() const;

        void SetInertia(bool enabled);
        bool HasInertia() const;

        void SetDecelerationRate(float rate);
        float GetDecelerationRate() const;

        void SetScrollSensitivity(float sensitivity);
        float GetScrollSensitivity() const;

        void SetContentSize(const glm::vec2& size);
        glm::vec2 GetContentSize() const;

        void SetScrollPosition(const glm::vec2& position);
        glm::vec2 GetScrollPosition() const;

        void SetNormalizedPosition(const glm::vec2& position);
        glm::vec2 GetNormalizedPosition() const;

        void SetBackgroundColor(const glm::vec4& color);
        glm::vec4 GetBackgroundColor() const;

        void SetScrollbarColor(const glm::vec4& color);
        glm::vec4 GetScrollbarColor() const;

        void SetScrollbarWidth(float width);
        float GetScrollbarWidth() const;

        void SetScrollbarVisibility(EScrollbarVisibility visibility);
        EScrollbarVisibility GetScrollbarVisibility() const;

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;
        void OnPointerDown() override;
        void OnPointerUp() override;

        /// @brief Called when scroll position changes
        ScrollCallback OnValueChanged;

        /// @brief Get the content offset for child widgets
        glm::vec2 GetContentOffset() const;

    private:
        void UpdateScrollbars();
        void ClampScrollPosition();
        glm::vec2 GetViewportSize() const;

        bool horizontalScrollEnabled = true;
        bool verticalScrollEnabled = true;
        
        EScrollMovementType movementType = EScrollMovementType::ELASTIC;
        float elasticity = 0.1f;
        
        bool inertia = true;
        float decelerationRate = 0.135f;
        float scrollSensitivity = 1.0f;
        
        glm::vec2 contentSize = glm::vec2(500.0f, 1000.0f);
        glm::vec2 scrollPosition = glm::vec2(0.0f);
        glm::vec2 velocity = glm::vec2(0.0f);
        
        glm::vec4 backgroundColor = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
        glm::vec4 scrollbarColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.8f);
        float scrollbarWidth = 10.0f;
        
        EScrollbarVisibility scrollbarVisibility = EScrollbarVisibility::AUTO_HIDE;
        
        bool isDragging = false;
        glm::vec2 dragStartPos = glm::vec2(0.0f);
        glm::vec2 dragStartScroll = glm::vec2(0.0f);
        
        float scrollbarFadeAlpha = 0.0f;
        float scrollbarFadeTimer = 0.0f;
    };

} // namespace golias
