#include "scene/components/widget/canvas_component.h"

#include "core/engine.h"
#include "font/font.h"
#include "graphics/texture.h"
#include "graphics/vertex_layout.h"
#include "render/mesh.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/components/widget/widget_component.h"
#include "scene/game_object.h"

namespace golias {

    bool CanvasComponent::LoadProperties(const Json& properties) {

        return true;
    }

    void CanvasComponent::Start() {
        VertexLayout layout;
        layout.Elements.push_back({0, 2, GL_FLOAT, 0});
        layout.Elements.push_back({1, 4, GL_FLOAT, 4 * sizeof(float)});
        layout.Elements.push_back({2, 2, GL_FLOAT, 2 * sizeof(float)});
        layout.Stride = 8 * sizeof(float);

        const std::vector<float> initialVertices(4 * 8, 0.0f);
        const std::vector<uint32_t> initialIndices = {0, 1, 2, 0, 2, 3};
        mMesh                                      = std::make_shared<Mesh>(layout, initialVertices, initialIndices);
    }

    void CanvasComponent::Begin() {
        mBatches.clear();
        mVertices.clear();
        mIndices.clear();
    }

    void CanvasComponent::End() {
        if (!mMesh || mIndices.empty()) {
            return;
        }

        mMesh->Update(mVertices, mIndices);

        const Viewport& viewport = Engine::GetInstance().GetGraphicsDevice().GetViewport();

        RenderCanvasCommand command = {
            .Mesh     = mMesh.get(),
            .Batches  = mBatches,
            .Viewport = viewport,
        };

        Engine::GetInstance().GetCommandQueue().Submit(command);
    }

    void CanvasComponent::ProcessInput() {


        const InputManager& inputManager = Engine::GetInstance().GetInputManager();

        bool mouseDown     = inputManager.IsMouseButtonPressed(MouseButton::Left);
        bool mouseUp       = inputManager.IsMouseButtonReleased(MouseButton::Left);
        bool mouseReleased = inputManager.IsMouseButtonReleased(MouseButton::Left);

        glm::vec2 mousePosition = inputManager.GetMousePosition();

        std::vector<WidgetComponent*> widgets;
        const auto& children = GetOwner()->GetChildren();

        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            const auto& child = *it;

            if (auto comp = child->GetComponent<WidgetComponent>()) {
                Collect(comp, widgets);
            }
        }

        WidgetComponent* hit = nullptr;
        for (auto element : widgets) {
            if (element->IsTopmost() && element->HitTest(mousePosition)) {
                hit = element;
                break;
            }
        }

        if (!hit) {
            for (auto element : widgets) {
                if (!element->IsTopmost() && element->HitTest(mousePosition)) {
                    hit = element;
                    break;
                }
            }
        }

        if (hit != mHovered) {
            if (mHovered) {
                mHovered->OnPointerExit();
            }

            mHovered = hit;

            if (mHovered) {
                mHovered->OnPointerEnter();
            }
            mPressed = nullptr;
        }

        if (!mPressed) {
            if (mouseDown && mHovered) {
                mPressed = mHovered;
                mPressed->OnPointerDown();
            }
        }

        if (mouseUp) {
            if (mPressed) {
                mPressed->OnPointerUp();

                if (mPressed == mHovered) {
                    mPressed->OnClick();
                }
            }

            mPressed = nullptr;
        }
    }

    void CanvasComponent::Collect(WidgetComponent* widget, std::vector<WidgetComponent*>& out) {
        if (!widget || !widget->GetOwner()) {
            return;
        }

        out.push_back(widget);

        for (const auto& child : widget->GetOwner()->GetChildren()) {
            if (!child->IsActive()) {
                continue;
            }

            if (auto* childWidget = child->GetComponent<WidgetComponent>()) {
                Collect(childWidget, out);
            }
        }
    }

    void CanvasComponent::Update(float deltaTime) {
        UNUSED_PARAMETER(deltaTime);

        if (RectTransformComponent* rt = GetOwner()->GetComponent<RectTransformComponent>()) {
            const GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();
            const Viewport& viewport     = device.GetViewport();
            rt->SetSize(glm::vec2(static_cast<float>(viewport.Width), static_cast<float>(viewport.Height)));
        }

        ProcessInput();

        Begin();

        std::vector<WidgetComponent*> widgets;
        const auto& children = GetOwner()->GetChildren();

        for (const auto& child : children) {
            if (!child->IsActive()) {
                continue;
            }

            if (auto comp = child->GetComponent<WidgetComponent>()) {
                Collect(comp, widgets);
            }
        }

        for (auto* widget : widgets) {
            if (!widget->IsTopmost()) {
                widget->Render(this);
            }
        }

        for (auto* widget : widgets) {
            if (widget->IsTopmost()) {
                widget->Render(this);
            }
        }

        End();
    }

    void CanvasComponent::Render(WidgetComponent* widget) {
        if (!widget || !widget->GetOwner()) {
            return;
        }

        widget->Render(this);

        for (const auto& child : widget->GetOwner()->GetChildren()) {
            if (auto* childWidget = child->GetComponent<WidgetComponent>()) {
                Render(childWidget);
            }
        }
    }

    void CanvasComponent::DrawQuad(const glm::vec2& lowerLeft, const glm::vec2& upperRight, const glm::vec4& color) {
        const uint32_t startIndex = static_cast<uint32_t>(mVertices.size() / 8);

        Texture* texture = nullptr;

        // clang-format off
        mVertices.insert(mVertices.end(), {
            lowerLeft.x, lowerLeft.y, 0.0f, 0.0f, color.r, color.g, color.b, color.a,
            upperRight.x, lowerLeft.y, 1.0f, 0.0, color.r, color.g, color.b, color.a,
            upperRight.x, upperRight.y, 1.0f, 1.0f, color.r, color.g, color.b, color.a,
            lowerLeft.x, upperRight.y, 0.0f, 1.0f, color.r, color.g, color.b, color.a,
        });
        // clang-format on

        mIndices.insert(mIndices.end(), {startIndex, startIndex + 1, startIndex + 2, startIndex, startIndex + 2, startIndex + 3});
        UpdateBatches(texture);
    }


    void CanvasComponent::DrawQuad(const glm::vec2& lowerLeft,
                                   const glm::vec2& upperRight,
                                   const glm::vec2& lowerLeftUV,
                                   const glm::vec2& upperRightUV,
                                   Texture* texture,
                                   const glm::vec4& color) {

        const uint32_t startIndex = static_cast<uint32_t>(mVertices.size() / 8);

        // clang-format off
        mVertices.insert(mVertices.end(), {
            lowerLeft.x, lowerLeft.y, lowerLeftUV.x, lowerLeftUV.y, color.r, color.g, color.b, color.a,
            upperRight.x, lowerLeft.y, upperRightUV.x, lowerLeftUV.y, color.r, color.g, color.b, color.a,
            upperRight.x, upperRight.y, upperRightUV.x, upperRightUV.y, color.r, color.g, color.b, color.a,
            lowerLeft.x, upperRight.y, lowerLeftUV.x, upperRightUV.y, color.r, color.g, color.b, color.a,
        });
        // clang-format on

        mIndices.insert(mIndices.end(), {startIndex, startIndex + 1, startIndex + 2, startIndex, startIndex + 2, startIndex + 3});
        UpdateBatches(texture);
    }

    void CanvasComponent::UpdateBatches(Texture* texture) {
        if (mBatches.empty() || mBatches.back().Texture != texture) {
            mBatches.push_back({texture, 6});
        } else {
            mBatches.back().IndexCount += 6;
        }
    }

    void CanvasComponent::DrawText(Font* font, const glm::vec2& origin, const String& text, const glm::vec4& color) {
        DrawText(font, origin, text, color, nullptr);
    }

    void CanvasComponent::DrawText(
        Font* font, const glm::vec2& origin, const String& text, const glm::vec4& color, const glm::vec4* outlineColor) {
        if (!font || text.empty()) {
            return;
        }

        const TextureDesc& texDesc = font->GetTexture()->GetDesc();
        const float invWidth       = 1.0f / static_cast<float>(texDesc.Width);
        const float invHeight      = 1.0f / static_cast<float>(texDesc.Height);

        const float baseBaselineY = origin.y + static_cast<float>(font->GetAscent());
        const float lineHeight    = static_cast<float>(font->GetLineHeight());

        float cursorX   = origin.x;
        float baselineY = baseBaselineY;

        for (size_t i = 0; i < text.size(); ++i) {
            const char c = text[i];

            if (c == '\n') {
                cursorX = origin.x;
                baselineY += lineHeight;
                continue;
            }

            if (c == '\r') {
                if (i + 1 < text.size() && text[i + 1] == '\n') {
                    continue;
                }

                cursorX = origin.x;
                continue;
            }

            const auto& desc = font->GetGlyphDescription(c);

            const float x1 = cursorX + static_cast<float>(desc.OffsetX);
            const float y1 = baselineY + static_cast<float>(desc.OffsetY);
            const float x2 = x1 + static_cast<float>(desc.Width);
            const float y2 = y1 + static_cast<float>(desc.Height);

            const float u1 = (static_cast<float>(desc.X0)) * invWidth;
            const float v1 = (static_cast<float>(desc.Y0)) * invHeight;
            const float u2 = (static_cast<float>(desc.X1)) * invWidth;
            const float v2 = (static_cast<float>(desc.Y1)) * invHeight;

            cursorX += static_cast<float>(desc.Advance);

            if (outlineColor && outlineColor->a > 0.0f) {
                DrawQuad(
                    glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec2(u1, v1), glm::vec2(u2, v2), font->GetTexture().get(), *outlineColor);
            }

            DrawQuad(glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec2(u1, v1), glm::vec2(u2, v2), font->GetTexture().get(), color);
        }
    }

} // namespace golias
