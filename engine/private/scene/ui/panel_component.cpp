#include "scene/ui/panel_component.h"

#include "core/engine.h"
#include "core/graphics/texture_2d.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void PanelWidgetComponent::SetColor(const glm::vec4& c) {
        color = c;
    }

    glm::vec4 PanelWidgetComponent::GetColor() const {
        return color;
    }

    void PanelWidgetComponent::SetTexture(const std::shared_ptr<Texture2D>& pTexture) {
        texture = pTexture;
    }

    const std::shared_ptr<Texture2D>& PanelWidgetComponent::GetTexture() const {
        return texture;
    }

    void PanelWidgetComponent::SetRaycastTarget(bool enabled) {
        raycastTarget = enabled;
    }

    bool PanelWidgetComponent::IsRaycastTarget() const {
        return raycastTarget;
    }

    void PanelWidgetComponent::Start() {
    }

    void PanelWidgetComponent::Update(float deltaTime) {
    }

    void PanelWidgetComponent::Draw(CanvasComponent* pCanvas) {
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

        Texture2D* texPtr = texture ? texture.get() : nullptr;

        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            texPtr,
            color);
    }

    void PanelWidgetComponent::LoadProperties(const nlohmann::json& json) {
        if (json.contains("color")) {
            const auto& col = json["color"];
            color = {col.value("r", 0.0f), col.value("g", 0.0f), col.value("b", 0.0f), col.value("a", 0.5f)};
        }

        if (json.contains("texture")) {
            std::string texPath = json["texture"].get<std::string>();
            texture = Texture2D::Load(texPath);
        }

        raycastTarget = json.value("raycastTarget", true);
    }

    bool PanelWidgetComponent::HitTest(const glm::vec2& point) const {
        if (!raycastTarget) {
            return false;
        }

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

} // namespace golias
