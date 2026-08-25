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
            const auto& meshObj = properties["mesh"];

            const String type = meshObj["type"];
            if (type == "cube") {
                glm::vec3 size = glm::vec3(1.0f);
                if (meshObj.contains("size")) {
                    const auto& sizeObj = meshObj["size"];
                    size.x              = sizeObj.value("x", 1.0f);
                    size.y              = sizeObj.value("y", 1.0f);
                    size.z              = sizeObj.value("z", 1.0f);
                }

                Ref<Mesh> cubeMesh = Mesh::CreateCube(size);
                mMesh              = cubeMesh;

            } else if (type == "sphere") {
                float radius = meshObj.value("radius", 1.0f);

                Ref<Mesh> sphere = Mesh::CreateSphere(radius);
                mMesh            = sphere;

            } else if (type == "capsule") {
                float radius = meshObj.value("radius", 0.4f);
                float height = meshObj.value("height", 1.2f);

                Ref<Mesh> capsule = Mesh::CreateCapsule(radius, height);
                mMesh             = capsule;
            }
        } else {
            GOLIAS_ASSERT_MSG(false, "StaticMeshComponent: Unsupported mesh type in JSON.");
        }

        if (properties.contains("material")) {
            const auto& materialObj = properties["material"];
            if (materialObj.is_object() && materialObj.contains("path")) {
                String path = materialObj["path"].get<String>();
                mMaterial   = Material::Load(path);
            }
        } else {
            GOLIAS_ASSERT_MSG(false, "StaticMeshComponent: Missing 'material' property in JSON.");
        }

        return true;
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
