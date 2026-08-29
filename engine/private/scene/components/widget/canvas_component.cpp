#include "scene/components/widget/canvas_component.h"

#include "core/engine.h"
#include "graphics/texture.h"
#include "graphics/vertex_layout.h"
#include "render/mesh.h"
#include "scene/components/widget/widget_component.h"
#include "scene/game_object.h"

namespace golias {

    void CanvasComponent::Start() {
        VertexLayout layout;
        layout.Elements.push_back({0, 2, GL_FLOAT, 0});
        layout.Elements.push_back({1, 2, GL_FLOAT, 2 * sizeof(float)});
        layout.Elements.push_back({2, 4, GL_FLOAT, 4 * sizeof(float)});
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

    void CanvasComponent::Update(float deltaTime) {
        UNUSED_PARAMETER(deltaTime);

        Begin();

        if (GetOwner()) {
            for (const auto& child : GetOwner()->GetChildren()) {
                if (auto* widget = child->GetComponent<WidgetComponent>()) {
                    Render(widget);
                }
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

} // namespace golias
