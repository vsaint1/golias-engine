#include "scene/ui/scrollrect_component.h"

#include "core/engine.h"
#include "core/input/input_manager.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void ScrollRectWidgetComponent::SetHorizontalScrollEnabled(bool enabled) {
        horizontalScrollEnabled = enabled;
    }

    bool ScrollRectWidgetComponent::IsHorizontalScrollEnabled() const {
        return horizontalScrollEnabled;
    }

    void ScrollRectWidgetComponent::SetVerticalScrollEnabled(bool enabled) {
        verticalScrollEnabled = enabled;
    }

    bool ScrollRectWidgetComponent::IsVerticalScrollEnabled() const {
        return verticalScrollEnabled;
    }

    void ScrollRectWidgetComponent::SetMovementType(EScrollMovementType type) {
        movementType = type;
    }

    EScrollMovementType ScrollRectWidgetComponent::GetMovementType() const {
        return movementType;
    }

    void ScrollRectWidgetComponent::SetElasticity(float e) {
        elasticity = e;
    }

    float ScrollRectWidgetComponent::GetElasticity() const {
        return elasticity;
    }

    void ScrollRectWidgetComponent::SetInertia(bool enabled) {
        inertia = enabled;
    }

    bool ScrollRectWidgetComponent::HasInertia() const {
        return inertia;
    }

    void ScrollRectWidgetComponent::SetDecelerationRate(float rate) {
        decelerationRate = rate;
    }

    float ScrollRectWidgetComponent::GetDecelerationRate() const {
        return decelerationRate;
    }

    void ScrollRectWidgetComponent::SetScrollSensitivity(float sensitivity) {
        scrollSensitivity = sensitivity;
    }

    float ScrollRectWidgetComponent::GetScrollSensitivity() const {
        return scrollSensitivity;
    }

    void ScrollRectWidgetComponent::SetContentSize(const glm::vec2& size) {
        contentSize = size;
    }

    glm::vec2 ScrollRectWidgetComponent::GetContentSize() const {
        return contentSize;
    }

    void ScrollRectWidgetComponent::SetScrollPosition(const glm::vec2& position) {
        glm::vec2 oldPos = scrollPosition;
        scrollPosition = position;
        ClampScrollPosition();
        
        if (scrollPosition != oldPos && OnValueChanged) {
            OnValueChanged(scrollPosition);
        }
    }

    glm::vec2 ScrollRectWidgetComponent::GetScrollPosition() const {
        return scrollPosition;
    }

    void ScrollRectWidgetComponent::SetNormalizedPosition(const glm::vec2& position) {
        glm::vec2 viewportSize = GetViewportSize();
        glm::vec2 maxScroll = glm::max(contentSize - viewportSize, glm::vec2(0.0f));
        SetScrollPosition(position * maxScroll);
    }

    glm::vec2 ScrollRectWidgetComponent::GetNormalizedPosition() const {
        glm::vec2 viewportSize = GetViewportSize();
        glm::vec2 maxScroll = glm::max(contentSize - viewportSize, glm::vec2(0.0f));
        
        glm::vec2 normalized;
        normalized.x = maxScroll.x > 0.0f ? scrollPosition.x / maxScroll.x : 0.0f;
        normalized.y = maxScroll.y > 0.0f ? scrollPosition.y / maxScroll.y : 0.0f;
        
        return glm::clamp(normalized, glm::vec2(0.0f), glm::vec2(1.0f));
    }

    void ScrollRectWidgetComponent::SetBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }

    glm::vec4 ScrollRectWidgetComponent::GetBackgroundColor() const {
        return backgroundColor;
    }

    void ScrollRectWidgetComponent::SetScrollbarColor(const glm::vec4& color) {
        scrollbarColor = color;
    }

    glm::vec4 ScrollRectWidgetComponent::GetScrollbarColor() const {
        return scrollbarColor;
    }

    void ScrollRectWidgetComponent::SetScrollbarWidth(float width) {
        scrollbarWidth = width;
    }

    float ScrollRectWidgetComponent::GetScrollbarWidth() const {
        return scrollbarWidth;
    }

    void ScrollRectWidgetComponent::SetScrollbarVisibility(EScrollbarVisibility visibility) {
        scrollbarVisibility = visibility;
    }

    EScrollbarVisibility ScrollRectWidgetComponent::GetScrollbarVisibility() const {
        return scrollbarVisibility;
    }

    void ScrollRectWidgetComponent::Start() {
    }

    void ScrollRectWidgetComponent::Update(float deltaTime) {
        auto& input = Engine::GetInstance().GetInputManager();
        
        // Handle mouse wheel scrolling
        float wheelDelta = input.GetMouseWheelDelta();
        if (std::abs(wheelDelta) > 0.001f) {
            // Check if mouse is over this widget
            glm::vec2 mousePos = input.GetMousePosition();
            auto& renderer = Engine::GetInstance().GetSceneRenderer();
            const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
            mousePos.y = viewport.height - mousePos.y;
            
            if (HitTest(mousePos)) {
                glm::vec2 scrollDelta(0.0f);
                
                if (verticalScrollEnabled) {
                    scrollDelta.y = wheelDelta * scrollSensitivity * 50.0f;
                } else if (horizontalScrollEnabled) {
                    scrollDelta.x = wheelDelta * scrollSensitivity * 50.0f;
                }
                
                SetScrollPosition(scrollPosition - scrollDelta);
                velocity = glm::vec2(0.0f);
                
                scrollbarFadeAlpha = 1.0f;
                scrollbarFadeTimer = 0.0f;
            }
        }
        
        // Handle dragging
        if (isDragging) {
            glm::vec2 mousePos = input.GetMousePosition();
            auto& renderer = Engine::GetInstance().GetSceneRenderer();
            const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
            mousePos.y = viewport.height - mousePos.y;
            
            glm::vec2 delta = mousePos - dragStartPos;
            glm::vec2 newPos = dragStartScroll - delta;
            
            if (inertia) {
                velocity = (newPos - scrollPosition) / std::max(deltaTime, 0.001f);
            }
            
            SetScrollPosition(newPos);
            
            scrollbarFadeAlpha = 1.0f;
            scrollbarFadeTimer = 0.0f;
        }
        
        // Apply inertia
        if (!isDragging && inertia && glm::length(velocity) > 1.0f) {
            SetScrollPosition(scrollPosition + velocity * deltaTime);
            velocity *= (1.0f - decelerationRate);
            
            scrollbarFadeAlpha = 1.0f;
            scrollbarFadeTimer = 0.0f;
        }
        
        // Handle elastic bounds
        if (movementType == EScrollMovementType::ELASTIC && !isDragging) {
            glm::vec2 viewportSize = GetViewportSize();
            glm::vec2 maxScroll = glm::max(contentSize - viewportSize, glm::vec2(0.0f));
            
            glm::vec2 targetPos = scrollPosition;
            
            if (scrollPosition.x < 0.0f) {
                targetPos.x = 0.0f;
            } else if (scrollPosition.x > maxScroll.x) {
                targetPos.x = maxScroll.x;
            }
            
            if (scrollPosition.y < 0.0f) {
                targetPos.y = 0.0f;
            } else if (scrollPosition.y > maxScroll.y) {
                targetPos.y = maxScroll.y;
            }
            
            if (targetPos != scrollPosition) {
                scrollPosition = glm::mix(scrollPosition, targetPos, elasticity);
            }
        }
        
        // Handle scrollbar fade
        if (scrollbarVisibility == EScrollbarVisibility::AUTO_HIDE) {
            scrollbarFadeTimer += deltaTime;
            if (scrollbarFadeTimer > 1.0f) {
                scrollbarFadeAlpha = std::max(0.0f, scrollbarFadeAlpha - deltaTime * 2.0f);
            }
        } else if (scrollbarVisibility == EScrollbarVisibility::PERMANENT) {
            scrollbarFadeAlpha = 1.0f;
        }
    }

    void ScrollRectWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (!pCanvas) {
            return;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return;
        }

        auto pos = rt->GetScreenPosition();
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;

        // Draw background
        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            backgroundColor);

        // Draw scrollbars
        if (scrollbarFadeAlpha > 0.01f) {
            glm::vec2 viewportSize = scaledSize;
            glm::vec2 scaledContentSize = contentSize * uiScale;
            float scaledScrollbarWidth = scrollbarWidth * uiScale;
            
            glm::vec4 barColor = scrollbarColor;
            barColor.a *= scrollbarFadeAlpha;
            
            // Vertical scrollbar
            if (verticalScrollEnabled && scaledContentSize.y > viewportSize.y) {
                float scrollbarHeight = (viewportSize.y / scaledContentSize.y) * viewportSize.y;
                scrollbarHeight = std::max(scrollbarHeight, 20.0f * uiScale);
                
                float maxScrollY = scaledContentSize.y - viewportSize.y;
                float normalizedY = maxScrollY > 0.0f ? (scrollPosition.y * uiScale) / maxScrollY : 0.0f;
                float scrollbarY = pos.y + (viewportSize.y - scrollbarHeight) * (1.0f - normalizedY);
                
                float scrollbarX = pos.x + scaledSize.x - scaledScrollbarWidth;
                
                pCanvas->DrawTexture2D(
                    glm::vec3(scrollbarX, scrollbarY, 0.02f),
                    glm::vec3(scrollbarX + scaledScrollbarWidth, scrollbarY + scrollbarHeight, 0.02f),
                    glm::vec2(0.0f),
                    glm::vec2(1.0f),
                    nullptr,
                    barColor);
            }
            
            // Horizontal scrollbar
            if (horizontalScrollEnabled && scaledContentSize.x > viewportSize.x) {
                float scrollbarWidth_ = (viewportSize.x / scaledContentSize.x) * viewportSize.x;
                scrollbarWidth_ = std::max(scrollbarWidth_, 20.0f * uiScale);
                
                float maxScrollX = scaledContentSize.x - viewportSize.x;
                float normalizedX = maxScrollX > 0.0f ? (scrollPosition.x * uiScale) / maxScrollX : 0.0f;
                float scrollbarX = pos.x + (viewportSize.x - scrollbarWidth_) * normalizedX;
                
                pCanvas->DrawTexture2D(
                    glm::vec3(scrollbarX, pos.y, 0.02f),
                    glm::vec3(scrollbarX + scrollbarWidth_, pos.y + scaledScrollbarWidth, 0.02f),
                    glm::vec2(0.0f),
                    glm::vec2(1.0f),
                    nullptr,
                    barColor);
            }
        }
    }

    void ScrollRectWidgetComponent::LoadProperties(const nlohmann::json& json) {
        horizontalScrollEnabled = json.value("horizontalScrollEnabled", true);
        verticalScrollEnabled = json.value("verticalScrollEnabled", true);
        elasticity = json.value("elasticity", 0.1f);
        inertia = json.value("inertia", true);
        decelerationRate = json.value("decelerationRate", 0.135f);
        scrollSensitivity = json.value("scrollSensitivity", 1.0f);
        scrollbarWidth = json.value("scrollbarWidth", 10.0f);

        if (json.contains("contentSize")) {
            const auto& size = json["contentSize"];
            contentSize = {size.value("x", 500.0f), size.value("y", 1000.0f)};
        }

        if (json.contains("backgroundColor")) {
            const auto& col = json["backgroundColor"];
            backgroundColor = {col.value("r", 0.15f), col.value("g", 0.15f), col.value("b", 0.15f), col.value("a", 1.0f)};
        }

        if (json.contains("scrollbarColor")) {
            const auto& col = json["scrollbarColor"];
            scrollbarColor = {col.value("r", 0.4f), col.value("g", 0.4f), col.value("b", 0.4f), col.value("a", 0.8f)};
        }
    }

    bool ScrollRectWidgetComponent::HitTest(const glm::vec2& point) const {
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return false;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto p1 = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        auto p2 = p1 + scaledSize;

        return (point.x >= p1.x) && (point.x <= p2.x) && (point.y >= p1.y) && (point.y <= p2.y);
    }

    void ScrollRectWidgetComponent::OnPointerDown() {
        isDragging = true;
        
        auto& input = Engine::GetInstance().GetInputManager();
        dragStartPos = input.GetMousePosition();
        
        auto& renderer = Engine::GetInstance().GetSceneRenderer();
        const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
        dragStartPos.y = viewport.height - dragStartPos.y;
        
        dragStartScroll = scrollPosition;
        velocity = glm::vec2(0.0f);
    }

    void ScrollRectWidgetComponent::OnPointerUp() {
        isDragging = false;
    }

    glm::vec2 ScrollRectWidgetComponent::GetContentOffset() const {
        return -scrollPosition;
    }

    void ScrollRectWidgetComponent::ClampScrollPosition() {
        if (movementType == EScrollMovementType::CLAMPED) {
            glm::vec2 viewportSize = GetViewportSize();
            glm::vec2 maxScroll = glm::max(contentSize - viewportSize, glm::vec2(0.0f));
            
            scrollPosition = glm::clamp(scrollPosition, glm::vec2(0.0f), maxScroll);
        }
    }

    glm::vec2 ScrollRectWidgetComponent::GetViewportSize() const {
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return glm::vec2(0.0f);
        }
        return rt->GetSize();
    }

} // namespace golias
