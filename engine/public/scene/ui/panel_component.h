#pragma once
#include "scene/ui/ui_base.h"
#include <glm/vec4.hpp>

namespace golias {

    class Texture2D;

    /// @brief  Panel component for grouping UI elements
    class PanelWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(PanelWidgetComponent, WidgetComponent)
    public:
        void SetColor(const glm::vec4& color);
        glm::vec4 GetColor() const;

        void SetTexture(const std::shared_ptr<Texture2D>& pTexture);
        const std::shared_ptr<Texture2D>& GetTexture() const;

        void SetRaycastTarget(bool enabled);
        bool IsRaycastTarget() const;

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;

    private:
        glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
        std::shared_ptr<Texture2D> texture = nullptr;
        bool raycastTarget = true;
    };

} // namespace golias
