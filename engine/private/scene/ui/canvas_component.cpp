#include "scene/ui/canvas_component.h"

#include "core/engine.h"
#include "scene/game_object.h"

namespace golias {


    void CanvasComponent::SetUseBillboarding(bool enable) {
        useBillboarding = enable;
    }

    bool CanvasComponent::GetUseBillboarding() const {
        return useBillboarding;
    }
    
    void CanvasComponent::Start() {

        vertices.clear();
        indices.clear();
        batches.clear();

        VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0                },
            {1, 4, EDataType::FLOAT, false, 3 * sizeof(float)},
            {2, 2, EDataType::FLOAT, false, 7 * sizeof(float)}
        };

        layout.stride = 9 * sizeof(float);

        mesh = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice()->CreateMeshFromData(layout, vertices, indices);
        if (!mesh) {
            spdlog::error("CanvasComponent::Start Failed to create canvas mesh.");
        }
    }


    void CanvasComponent::Update(float deltaTime) {

        vertices.clear();
        indices.clear();
        batches.clear();

        const auto& children = GetOwner()->GetChildren();

        for (const auto& child : children) {
            if (auto pComponent = child->GetComponent<WidgetComponent>()) {
                Draw(pComponent);
            }
        }

        if (!vertices.empty() && !indices.empty() && !batches.empty()) {
            mesh->Update(vertices, indices);

            if (canvasMode == ECanvasMode::SCREEN_SPACE) {
                ScreenCanvasCommand command;
                command.mesh    = mesh.get();
                command.batches = batches;
                Engine::GetInstance().GetSceneRenderer().Submit(command);
            } else {
                WorldCanvasCommand command;
                command.mesh            = mesh.get();
                command.batches         = batches;
                command.modelMatrix     = GetOwner()->GetWorldTransform();
                command.useBillboarding = useBillboarding;
                command.scale           = worldSpaceScale;
                Engine::GetInstance().GetSceneRenderer().Submit(command);
            }
        }
    }

    void CanvasComponent::Draw(WidgetComponent* pWidget) {

        if (!pWidget) {
            return;
        }

        pWidget->Draw(this);

        const auto& children = pWidget->GetOwner()->GetChildren();
        for (const auto& child : children) {
            if (auto pComponent = child->GetComponent<WidgetComponent>()) {
                Draw(pComponent);
            }
        }
    }

    void CanvasComponent::SetCanvasMode(ECanvasMode mode) {
        canvasMode = mode;
    }

    ECanvasMode CanvasComponent::GetCanvasMode() const {
        return canvasMode;
    }

    void CanvasComponent::SetWorldSpaceScale(float scale) {
        worldSpaceScale = scale;
    }

    float CanvasComponent::GetWorldSpaceScale() const {
        return worldSpaceScale;
    }

    void CanvasComponent::DrawTexture2D(
        const glm::vec3& p1, const glm::vec3& p2, const glm::vec2& uv1, const glm::vec2& uv2, Texture2D* pTexture, const glm::vec4& color) {

        Uint32 base = static_cast<Uint32>(vertices.size() / 9);

        vertices.insert(vertices.end(),
                        {// Position               // Color                     // UV
                         p1.x,    p1.y,    p1.z,    color.r, color.g, color.b, color.a, uv1.x,   uv1.y,   p2.x,    p1.y,    p1.z,
                         color.r, color.g, color.b, color.a, uv2.x,   uv1.y,   p2.x,    p2.y,    p2.z,    color.r, color.g, color.b,
                         color.a, uv2.x,   uv2.y,   p1.x,    p2.y,    p2.z,    color.r, color.g, color.b, color.a, uv1.x,   uv2.y});

        indices.insert(indices.end(), {base, base + 1, base + 2, base + 2, base + 3, base});

        if (batches.empty() || batches.back().texture != pTexture) {
            CanvasBatch batch;
            batch.texture    = pTexture;
            batch.indexCount = 6;
            batches.push_back(batch);
        } else {
            batches.back().indexCount += 6;
        }
    }

} // namespace golias
