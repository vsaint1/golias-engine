#pragma once
#include "core/graphics/scene_renderer.h"
#include "scene/ui/ui_base.h"

namespace golias {

    class Texture2D;

    class CanvasComponent : public Component {
        COMPONENT(CanvasComponent)
    public:
        void Start() override;

        void Update(float deltaTime) override;

        void Draw(WidgetComponent* pWidget);

        void DrawTexture2D(const glm::vec2& p1,
                           const glm::vec2& p2,
                           const glm::vec2& uv1,
                           const glm::vec2& uv2,
                           Texture2D* pTexture,
                           const glm::vec4& color);

    private:
        std::vector<CanvasBatch> batches;
        std::vector<float> vertices;
        std::vector<Uint32> indices;
        std::shared_ptr<Mesh> mesh = nullptr;
    };

} // namespace golias
