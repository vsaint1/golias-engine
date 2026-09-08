#pragma once

#include "render/command_queue.h"
#include "scene/components/component.h"

namespace golias {

    class WidgetComponent;
    class Texture;
    class Mesh;
    class Font;

    class CanvasComponent : public Component {
        COMPONENT(CanvasComponent)
    public:
        CanvasComponent()  = default;
        ~CanvasComponent() = default;

        void Begin();

        void End();

        bool LoadProperties(const Json& properties) override;

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

        void DrawText(Font* font, const glm::vec2& origin, const String& text, const glm::vec4& color);

        void DrawText(Font* font, const glm::vec2& origin, const String& text, const glm::vec4& color, const glm::vec4* outlineColor);

        void PushClip(const glm::vec2& lowerLeft, const glm::vec2& upperRight);
        void PopClip();

        void Collect(WidgetComponent* widget, std::vector<WidgetComponent*>& out);

    private:
        void UpdateBatches(Texture* texture);

        void RenderWidget(WidgetComponent* widget);

        void ProcessInput();

        static void
            GetWidgetContext(const WidgetComponent* widget, ScissorRect& clip, bool& hasClip, glm::vec2& contentOffset, bool& topmost);

    private:
        Ref<Mesh> mMesh = nullptr;

        std::vector<CanvasBatch> mBatches;
        std::vector<float> mVertices;
        std::vector<uint32_t> mIndices;

        ScissorRect mClipRect = {};
        bool mHasClip            = false;
        glm::vec2 mContentOffset = glm::vec2(0.0f);

        ScissorRect mSavedClip{};
        bool mSavedHasClip = false;

        WidgetComponent* mHovered = nullptr;
        WidgetComponent* mPressed = nullptr;
    };

} // namespace golias
