#pragma once
#include "core/graphics/scene_renderer.h"
#include "scene/ui/ui_base.h"

namespace golias {

    class Texture2D;

    /// @brief Defines the rendering mode of the canvas.
    enum class ECanvasMode {
        SCREEN_SPACE, /// 2D screen-space rendering (overlay)
        WORLD_SPACE /// 3D world rendering (with depth testing and transforms)
    };


    class CanvasComponent : public Component {
        COMPONENT(CanvasComponent)
    public:
        void Start() override;

        void Update(float deltaTime) override;

        void Draw(WidgetComponent* pWidget);

        void DrawTexture2D(const glm::vec3& p1,
                           const glm::vec3& p2,
                           const glm::vec2& uv1,
                           const glm::vec2& uv2,
                           Texture2D* pTexture,
                           const glm::vec4& color);

        void SetCanvasMode(ECanvasMode mode);
        ECanvasMode GetCanvasMode() const;

        void SetWorldSpaceScale(float scale);
        float GetWorldSpaceScale() const;

        void SetUseBillboarding(bool enable);
        bool GetUseBillboarding() const;

        void CollectWidget(WidgetComponent* pWidget, std::vector<WidgetComponent*>& outWidgets);


    private:
        std::vector<CanvasBatch> batches;
        std::vector<float> vertices;
        std::vector<Uint32> indices;
        std::shared_ptr<Mesh> mesh    = nullptr;
        ECanvasMode canvasMode     = ECanvasMode::SCREEN_SPACE;
        bool useBillboarding       = false;
        float worldSpaceScale      = 1.0f; // Scale factor for world space (1px = 0.01 units)
    };

} // namespace golias
