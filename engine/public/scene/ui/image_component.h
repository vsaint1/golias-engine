#pragma once
#include "scene/ui/ui_base.h"
#include <glm/vec4.hpp>

namespace golias {

    class Texture2D;

    /// @brief Image fill type
    enum class EImageType {
        SIMPLE,    /// Normal stretched image
        SLICED,    /// 9-slice scaling
        TILED,     /// Tiled/repeated
        FILLED     /// Radial or linear fill
    };

    /// @brief Fill method for FILLED type images
    enum class EFillMethod {
        HORIZONTAL,
        VERTICAL,
        RADIAL_90,
        RADIAL_180,
        RADIAL_360
    };

    /// @brief Fill origin direction
    enum class EFillOrigin {
        BOTTOM,
        RIGHT,
        TOP,
        LEFT
    };

    /// @brief  Image component for Canvas UI
    class ImageWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(ImageWidgetComponent, WidgetComponent)
    public:
        void SetTexture(const std::shared_ptr<Texture2D>& pTexture);
        const std::shared_ptr<Texture2D>& GetTexture() const;

        void SetColor(const glm::vec4& color);
        glm::vec4 GetColor() const;

        void SetImageType(EImageType type);
        EImageType GetImageType() const;

        void SetFillMethod(EFillMethod method);
        EFillMethod GetFillMethod() const;

        void SetFillOrigin(EFillOrigin origin);
        EFillOrigin GetFillOrigin() const;

        void SetFillAmount(float amount);
        float GetFillAmount() const;

        void SetFillClockwise(bool clockwise);
        bool IsFillClockwise() const;

        void SetPreserveAspect(bool preserve);
        bool GetPreserveAspect() const;

        /// @brief Enable/disable raycast target (for hit testing)
        void SetRaycastTarget(bool enabled);
        bool IsRaycastTarget() const;

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;

    private:
        std::shared_ptr<Texture2D> texture = nullptr;
        glm::vec4 color = glm::vec4(1.0f);
        
        EImageType imageType = EImageType::SIMPLE;
        EFillMethod fillMethod = EFillMethod::HORIZONTAL;
        EFillOrigin fillOrigin = EFillOrigin::LEFT;
        
        float fillAmount = 1.0f;
        bool fillClockwise = true;
        bool preserveAspect = false;
        bool raycastTarget = true;
    };

} // namespace golias
