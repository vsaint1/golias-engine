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

            } else if (type == "plane") {
                glm::vec2 size = glm::vec2(10.0f);
                if (meshObj.contains("size")) {
                    const auto& sizeObj = meshObj["size"];
                    size.x              = sizeObj.value("x", 10.0f);
                    size.y              = sizeObj.value("y", 10.0f);
                }

                if (meshObj.contains("subdivisions")) {
                    const auto& subdivisionsObj = meshObj["subdivisions"];
                    int subdivisionsX           = subdivisionsObj.value("x", 1);
                    int subdivisionsY           = subdivisionsObj.value("y", 1);
                    mMesh                       = Mesh::CreatePlane(size, subdivisionsX, subdivisionsY);
                }
            }

        } else {
            GOLIAS_ASSERT_MSG(false, "StaticMeshComponent: Unsupported mesh type in JSON.");
        }

        if (properties.contains("material")) {
            const auto& materialObj = properties["material"];
            if (materialObj.is_object() && materialObj.contains("path")) {
                String path = materialObj["path"].get<String>();

                // if (path == "__default__") {
                //     mMaterial = Material::CreateDefault();
                // }
                mMaterial = Engine::GetInstance().GetAssetManager().Load<Material>(path);

                if (mMaterial && materialObj.contains("override") && materialObj["override"].is_object()) {
                    const auto& materialOverride = materialObj["override"];

                    if (materialOverride.contains("parameters")) {
                        mMaterial = mMaterial->Clone();
                        mMaterial->ApplyParametersFromJson(materialOverride["parameters"]);
                    }
                }
            }
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
