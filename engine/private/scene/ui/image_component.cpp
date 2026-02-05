#include "scene/ui/image_component.h"

#include "core/engine.h"
#include "core/graphics/texture_2d.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    void ImageWidgetComponent::SetTexture(const std::shared_ptr<Texture2D>& pTexture) {
        texture = pTexture;
    }

    const std::shared_ptr<Texture2D>& ImageWidgetComponent::GetTexture() const {
        return texture;
    }

    void ImageWidgetComponent::SetColor(const glm::vec4& c) {
        color = c;
    }

    glm::vec4 ImageWidgetComponent::GetColor() const {
        return color;
    }

    void ImageWidgetComponent::SetImageType(EImageType type) {
        imageType = type;
    }

    EImageType ImageWidgetComponent::GetImageType() const {
        return imageType;
    }

    void ImageWidgetComponent::SetFillMethod(EFillMethod method) {
        fillMethod = method;
    }

    EFillMethod ImageWidgetComponent::GetFillMethod() const {
        return fillMethod;
    }

    void ImageWidgetComponent::SetFillOrigin(EFillOrigin origin) {
        fillOrigin = origin;
    }

    EFillOrigin ImageWidgetComponent::GetFillOrigin() const {
        return fillOrigin;
    }

    void ImageWidgetComponent::SetFillAmount(float amount) {
        fillAmount = glm::clamp(amount, 0.0f, 1.0f);
    }

    float ImageWidgetComponent::GetFillAmount() const {
        return fillAmount;
    }

    void ImageWidgetComponent::SetFillClockwise(bool clockwise) {
        fillClockwise = clockwise;
    }

    bool ImageWidgetComponent::IsFillClockwise() const {
        return fillClockwise;
    }

    void ImageWidgetComponent::SetPreserveAspect(bool preserve) {
        preserveAspect = preserve;
    }

    bool ImageWidgetComponent::GetPreserveAspect() const {
        return preserveAspect;
    }

    void ImageWidgetComponent::SetRaycastTarget(bool enabled) {
        raycastTarget = enabled;
    }

    bool ImageWidgetComponent::IsRaycastTarget() const {
        return raycastTarget;
    }

    void ImageWidgetComponent::Start() {
    }

    void ImageWidgetComponent::Update(float deltaTime) {
    }

    void ImageWidgetComponent::Draw(CanvasComponent* pCanvas) {
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

        glm::vec2 uv1(0.0f, 0.0f);
        glm::vec2 uv2(1.0f, 1.0f);

        // Handle filled image type
        if (imageType == EImageType::FILLED) {
            switch (fillMethod) {
                case EFillMethod::HORIZONTAL:
                    if (fillOrigin == EFillOrigin::LEFT) {
                        uv2.x = fillAmount;
                        scaledSize.x *= fillAmount;
                    } else if (fillOrigin == EFillOrigin::RIGHT) {
                        uv1.x = 1.0f - fillAmount;
                        pos.x += scaledSize.x * (1.0f - fillAmount);
                        scaledSize.x *= fillAmount;
                    }
                    break;
                case EFillMethod::VERTICAL:
                    if (fillOrigin == EFillOrigin::BOTTOM) {
                        uv2.y = fillAmount;
                        scaledSize.y *= fillAmount;
                    } else if (fillOrigin == EFillOrigin::TOP) {
                        uv1.y = 1.0f - fillAmount;
                        pos.y += scaledSize.y * (1.0f - fillAmount);
                        scaledSize.y *= fillAmount;
                    }
                    break;
                default:
                    // Radial fills would require more complex mesh generation
                    break;
            }
        }

        Texture2D* texPtr = texture ? texture.get() : nullptr;

        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            uv1,
            uv2,
            texPtr,
            color);
    }

    void ImageWidgetComponent::LoadProperties(const nlohmann::json& json) {
        if (json.contains("color")) {
            const auto& col = json["color"];
            color = {col.value("r", 1.0f), col.value("g", 1.0f), col.value("b", 1.0f), col.value("a", 1.0f)};
        }

        if (json.contains("texture")) {
            std::string texPath = json["texture"].get<std::string>();
            texture = Texture2D::Load(texPath);
        }

        if (json.contains("imageType")) {
            std::string typeStr = json["imageType"].get<std::string>();
            if (typeStr == "sliced") imageType = EImageType::SLICED;
            else if (typeStr == "tiled") imageType = EImageType::TILED;
            else if (typeStr == "filled") imageType = EImageType::FILLED;
            else imageType = EImageType::SIMPLE;
        }

        fillAmount = json.value("fillAmount", 1.0f);
        preserveAspect = json.value("preserveAspect", false);
        raycastTarget = json.value("raycastTarget", true);
    }

    bool ImageWidgetComponent::HitTest(const glm::vec2& point) const {
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
