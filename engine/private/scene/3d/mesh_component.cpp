#include "scene/3d/mesh_component.h"

#include "core/engine.h"
#include "core/graphics/rendering_canvas.h"

namespace golias {

    MeshComponent::MeshComponent(const std::shared_ptr<Mesh>& pMesh, const std::shared_ptr<Material>& pMaterial)
        : mesh(pMesh), material(pMaterial) {
            
    }

    void MeshComponent::Update(float deltaTime) {

        if (!mesh || !material) {
            return;
        }

        golias::DrawCommand command;
        command.mesh        = mesh.get();
        command.material    = material.get();
        command.modelMatrix = GetOwner()->GetWorldTransform();

        auto& rendering_canvas = golias::Engine::GetInstance().GetRenderingCanvas();
        rendering_canvas.Submit(command);
    }


} // namespace golias
