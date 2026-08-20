#include "scene/components/static_mesh_component.h"

#include "core/engine.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/game_object.h"

namespace golias {

    StaticMeshComponent::StaticMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material) : mMesh(mesh), mMaterial(material) {
    }

    void StaticMeshComponent::Update(float deltaTime) {

        if (!mMaterial || !mMesh) {
            return;
        }

        RenderCommand command;
        command.Mesh     = mMesh.get();
        command.Material = mMaterial.get();
        command.Model    = GetOwner()->GetWorldTransform();

        Engine::GetInstance().GetCommandQueue().Submit(command);
    }

    Ref<Mesh> StaticMeshComponent::GetMesh() const {
        return mMesh;
    }

    void StaticMeshComponent::SetMesh(const Ref<Mesh>& mesh) {
        mMesh = mesh;
    }

    Ref<Material> StaticMeshComponent::GetMaterial() const {
        return mMaterial;
    }

    void StaticMeshComponent::SetMaterial(const Ref<Material>& material) {
        mMaterial = material;
    }
} // namespace golias
