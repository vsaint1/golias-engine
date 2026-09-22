#include "scene/components/static_mesh_component.h"

#include "core/engine.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/game_object.h"

namespace golias {

    StaticMeshComponent::StaticMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material) : mMesh(mesh), mMaterial(material) {
    }

    bool StaticMeshComponent::LoadProperties(const Json& properties) {

        if (properties.contains("mesh")) {
            mMesh = Mesh::CreateFromJson(properties["mesh"]);

        } else {
            GOLIAS_ASSERT_MSG(false, "StaticMeshComponent: Unsupported mesh type in JSON.");
        }

        if (properties.contains("material") && properties["material"].is_object()) {
            mMaterial = Material::LoadFromJson(properties["material"]);
        } else {
            GOLIAS_ASSERT_MSG(false, "StaticMeshComponent: Missing 'material' property in JSON.");
        }

        return true;
    }

    bool StaticMeshComponent::SaveProperties(Json& properties) const {

        if (!mVisible) {
            properties["visible"] = false;
        }

        return true;
    }

    void StaticMeshComponent::Update(float deltaTime) {

        if (!mMaterial || !mMesh || !mVisible) {
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

    bool StaticMeshComponent::IsVisible() const {
        return mVisible;
    }

    void StaticMeshComponent::SetVisible(bool visible) {
        mVisible = visible;
    }

} // namespace golias
