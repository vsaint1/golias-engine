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
        layout.Elements.push_back({0, 2, VertexFormat::Float2, 0});
        layout.Elements.push_back({1, 4, VertexFormat::Float4, 4 * sizeof(float)});
        layout.Elements.push_back({2, 2, VertexFormat::Float2, 2 * sizeof(float)});
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

        // Widgets can be destroyed at runtime (e.g. a dropdown list closing). If the previously
        // hovered/pressed widget is no longer part of the canvas, drop the reference without
        // touching its memory - calling OnPointerExit/Up on freed objects would crash.
        if (mHovered && std::find(widgets.begin(), widgets.end(), mHovered) == widgets.end()) {
            mHovered = nullptr;
        }

        if (mPressed && std::find(widgets.begin(), widgets.end(), mPressed) == widgets.end()) {
            mPressed = nullptr;
        }

        WidgetComponent* hit = nullptr;
        auto hit_test_widget = [&](WidgetComponent* element, const glm::vec2& point, bool topmostOnly) {
            ScissorRect clip;
            bool hasClip = false;
            glm::vec2 contentOffset(0.0f);
            bool topmost = false;

            GetWidgetContext(element, clip, hasClip, contentOffset, topmost);

            if (topmost != topmostOnly) {
                return false;
            }

            // A masked widget can only be hit while the pointer is inside its mask.
            if (hasClip) {
                if (point.x < clip.X || point.x > clip.X + clip.Width || point.y < clip.Y || point.y > clip.Y + clip.Height) {
                    return false;
                }
            }

            return element->HitTest(point - contentOffset);
        };

        for (auto element : widgets) {
            if (hit_test_widget(element, mousePosition, true)) {
                hit = element;
                break;
            }
        }

        if (!hit) {
            for (auto element : widgets) {
                if (hit_test_widget(element, mousePosition, false)) {
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
            ScissorRect clip;
            bool hasClip = false;
            glm::vec2 contentOffset(0.0f);
            bool topmost = false;

            GetWidgetContext(widget, clip, hasClip, contentOffset, topmost);

            mClipRect      = clip;
            mHasClip       = hasClip;
            mContentOffset = contentOffset;

            if (!topmost) {
                RenderWidget(widget);
            }
        }

        mClipRect      = ScissorRect{};
        mHasClip       = false;
        mContentOffset = glm::vec2(0.0f);

        for (auto* widget : widgets) {
            ScissorRect clip;
            bool hasClip = false;
            glm::vec2 contentOffset(0.0f);
            bool topmost = false;

            GetWidgetContext(widget, clip, hasClip, contentOffset, topmost);

            mClipRect      = clip;
            mHasClip       = hasClip;
            mContentOffset = contentOffset;

            if (topmost) {
                RenderWidget(widget);
            }
        }

        mClipRect      = ScissorRect{};
        mHasClip       = false;
        mContentOffset = glm::vec2(0.0f);

        End();
    }

    void CanvasComponent::RenderWidget(WidgetComponent* widget) {
        if (!widget) {
            return;
        }

        // Cull widgets whose rect lies fully outside the current clip (e.g. rows scrolled out of a
        // mask viewport). This keeps per-frame canvas geometry proportional to what is on screen.
        if (RectTransformComponent* rectTransform = widget->GetOwner()->GetComponent<RectTransformComponent>()) {
            const glm::vec2 size = rectTransform->GetSize();
            const glm::vec2 ll   = rectTransform->GetScreenPosition() - rectTransform->GetPivot() * size + mContentOffset;
            const glm::vec2 ur   = ll + size;

            if (mHasClip) {
                const float clipRight  = mClipRect.X + mClipRect.Width;
                const float clipBottom = mClipRect.Y + mClipRect.Height;

                if (ur.x <= mClipRect.X || ll.x >= clipRight || ur.y <= mClipRect.Y || ll.y >= clipBottom) {
                    return;
                }
            }
        }

        widget->Render(this);
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
        const glm::vec2 ll = lowerLeft + mContentOffset;
        const glm::vec2 ur = upperRight + mContentOffset;

        const uint32_t startIndex = static_cast<uint32_t>(mVertices.size() / 8);

        Texture* texture = nullptr;

        // clang-format off
        mVertices.insert(mVertices.end(), {
            ll.x, ll.y, 0.0f, 0.0f, color.r, color.g, color.b, color.a,
            ur.x, ll.y, 1.0f, 0.0, color.r, color.g, color.b, color.a,
            ur.x, ur.y, 1.0f, 1.0f, color.r, color.g, color.b, color.a,
            ll.x, ur.y, 0.0f, 1.0f, color.r, color.g, color.b, color.a,
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

        const glm::vec2 ll = lowerLeft + mContentOffset;
        const glm::vec2 ur = upperRight + mContentOffset;

        const uint32_t startIndex = static_cast<uint32_t>(mVertices.size() / 8);

        // clang-format off
        mVertices.insert(mVertices.end(), {
            ll.x, ll.y, lowerLeftUV.x, lowerLeftUV.y, color.r, color.g, color.b, color.a,
            ur.x, ll.y, upperRightUV.x, lowerLeftUV.y, color.r, color.g, color.b, color.a,
            ur.x, ur.y, upperRightUV.x, upperRightUV.y, color.r, color.g, color.b, color.a,
            ll.x, ur.y, lowerLeftUV.x, upperRightUV.y, color.r, color.g, color.b, color.a,
        });
        // clang-format on

        mIndices.insert(mIndices.end(), {startIndex, startIndex + 1, startIndex + 2, startIndex, startIndex + 2, startIndex + 3});
        UpdateBatches(texture);
    }

    void CanvasComponent::PushClip(const glm::vec2& lowerLeft, const glm::vec2& upperRight) {
        mSavedClip     = mClipRect;
        mSavedHasClip  = mHasClip;

        const ScissorRect local = {
            std::min(lowerLeft.x, upperRight.x),
            std::min(lowerLeft.y, upperRight.y),
            std::abs(upperRight.x - lowerLeft.x),
            std::abs(upperRight.y - lowerLeft.y),
        };

        if (!mHasClip) {
            mClipRect = local;
            mHasClip  = true;
            return;
        }

        const float x1 = std::max(mClipRect.X, local.X);
        const float y1 = std::max(mClipRect.Y, local.Y);
        const float x2 = std::min(mClipRect.X + mClipRect.Width, local.X + local.Width);
        const float y2 = std::min(mClipRect.Y + mClipRect.Height, local.Y + local.Height);

        mClipRect = {x1, y1, std::max(x2 - x1, 0.0f), std::max(y2 - y1, 0.0f)};
    }

    void CanvasComponent::PopClip() {
        mClipRect    = mSavedClip;
        mHasClip     = mSavedHasClip;
    }

    void CanvasComponent::UpdateBatches(Texture* texture) {
        if (mBatches.empty() || mBatches.back().Texture != texture || mBatches.back().HasClip != mHasClip
            || (mHasClip && mBatches.back().ClipRect != mClipRect)) {
            mBatches.push_back({texture, 6, mClipRect, mHasClip});
        } else {
            mBatches.back().IndexCount += 6;
        }
    }

    void CanvasComponent::GetWidgetContext(
        const WidgetComponent* widget, ScissorRect& clip, bool& hasClip, glm::vec2& contentOffset, bool& topmost) {

        if (!widget || !widget->GetOwner()) {
            return;
        }

        topmost = widget->IsTopmost();

        glm::vec2 clipLowerLeft(0.0f);
        glm::vec2 clipUpperRight(0.0f);

        for (GameObject* ancestor = widget->GetOwner()->GetParent(); ancestor; ancestor = ancestor->GetParent()) {
            if (WidgetComponent* ancestorWidget = ancestor->GetComponent<WidgetComponent>()) {
                contentOffset += ancestorWidget->GetContentOffset();

                if (ancestorWidget->IsTopmost()) {
                    topmost = true;
                }

                glm::vec2 maskLowerLeft;
                glm::vec2 maskSize;
                if (ancestorWidget->GetMaskRect(maskLowerLeft, maskSize)) {
                    const glm::vec2 maskUpperRight = maskLowerLeft + maskSize;

                    if (!hasClip) {
                        hasClip        = true;
                        clipLowerLeft  = maskLowerLeft;
                        clipUpperRight = maskUpperRight;
                    } else {
                        clipLowerLeft.x  = std::max(clipLowerLeft.x, maskLowerLeft.x);
                        clipLowerLeft.y  = std::max(clipLowerLeft.y, maskLowerLeft.y);
                        clipUpperRight.x = std::min(clipUpperRight.x, maskUpperRight.x);
                        clipUpperRight.y = std::min(clipUpperRight.y, maskUpperRight.y);
                    }
                }
            }
        }

        if (hasClip) {
            clip = {
                clipLowerLeft.x,
                clipLowerLeft.y,
                std::max(clipUpperRight.x - clipLowerLeft.x, 0.0f),
                std::max(clipUpperRight.y - clipLowerLeft.y, 0.0f),
            };
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

        const glm::vec2 offsetOrigin = origin;

        const float baseBaselineY = offsetOrigin.y + static_cast<float>(font->GetAscent());
        const float lineHeight    = static_cast<float>(font->GetLineHeight());

        float cursorX   = offsetOrigin.x;
        float baselineY = baseBaselineY;

        for (size_t i = 0; i < text.size(); ++i) {
            const char c = text[i];

            if (c == '\n') {
                cursorX = offsetOrigin.x;
                baselineY += lineHeight;
                continue;
            }

            if (c == '\r') {
                if (i + 1 < text.size() && text[i + 1] == '\n') {
                    continue;
                }

                cursorX = offsetOrigin.x;
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
