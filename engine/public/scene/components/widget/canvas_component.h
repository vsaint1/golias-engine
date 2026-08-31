#pragma once

#include "render/command_queue.h"
#include "scene/components/component.h"

namespace golias {

    class WidgetComponent;
    class Texture;
    class Mesh;

    class CanvasComponent : public Component {
        COMPONENT(CanvasComponent)
    public:
        CanvasComponent()  = default;
        ~CanvasComponent() = default;

        
        void Begin();
        
        void End();
        
        bool LoadProperties(const Json& properties);
        
        void Start() override;

        void Update(float deltaTime) override;

        void Render(WidgetComponent* widget);

        void DrawQuad(const glm::vec2& lowerLeft, const glm::vec2& upperRight, const glm::vec4& color);

        void DrawQuad(const glm::vec2& lowerLeft,
                      const glm::vec2& upperRight,
                      const glm::vec2& lowerLeftUV,
                      const glm::vec2& upperRightUV,
                      Texture* texture,
                      const glm::vec4& color);

        void Collect(WidgetComponent* widget, std::vector<WidgetComponent*>& out);

    private:
        void UpdateBatches(Texture* texture);

        void ProcessInput();
    private:
        Ref<Mesh> mMesh = nullptr;

        std::vector<CanvasBatch> mBatches;
        std::vector<float> mVertices;
        std::vector<uint32_t> mIndices;

        WidgetComponent* mHovered = nullptr;
        WidgetComponent* mPressed = nullptr;
    };

} // namespace golias
