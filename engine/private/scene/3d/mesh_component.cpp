#include "scene/3d/mesh_component.h"

#include "core/engine.h"
#include "core/graphics/rendering_canvas.h"

namespace golias {

    MeshComponent::MeshComponent(const std::shared_ptr<Mesh>& pMesh, const std::shared_ptr<Material>& pMaterial)
        : mesh(pMesh), material(pMaterial) {
    }

    void MeshComponent::Start() {
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

    Mesh* MeshComponent::GetMesh() const {
        return mesh.get();
    }

    void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& pMesh) {
        mesh = pMesh;
    }

    void MeshComponent::SetMaterial(const std::shared_ptr<Material>& pMaterial) {
        material = pMaterial;
    }

    Material* MeshComponent::GetMaterial() const {
        return material.get();
    }


    void MeshComponent::LoadProperties(const nlohmann::json& json) {

        if (json.contains("material")) {
            const auto materialObject = json["material"];
            if (materialObject.contains("path")) {
                std::string materialPath = materialObject.value("path", "");
                
                const nlohmann::json* params = materialObject.contains("parameters") ? &materialObject["parameters"] : nullptr;
                auto mat = Material::Load(materialPath, params);
                
                if (mat) {
                    SetMaterial(mat);
                }
            }
          
        }

        if (json.contains("mesh")) {
            const auto& meshObj = json["mesh"];

            const std::string meshType = meshObj.value("type", "box");

            if (meshType == "box") {
                glm::vec3 extents = glm::vec3(1.0f);
                if(meshObj.contains("extents")){
                    const auto& extentsJson = meshObj["extents"];
                    extents.x               = extentsJson.value("x", 1.0f);
                    extents.y               = extentsJson.value("y", 1.0f);
                    extents.z               = extentsJson.value("z", 1.0f);
                }

                auto boxMesh = Mesh::CreateBox(extents);
                SetMesh(boxMesh);
            }

            if (meshType == "sphere") {
                float radius = 1.0f;
                uint32_t segments = 32;
                uint32_t rings = 32;

                if(meshObj.contains("radius")){
                    radius = meshObj.value("radius", 1.0f);
                }
                if(meshObj.contains("segments")){
                    segments = meshObj.value("segments", 32);
                }
                if(meshObj.contains("rings")){
                    rings = meshObj.value("rings", 32);
                }

                auto sphereMesh = Mesh::CreateSphere(radius, segments, rings);
                SetMesh(sphereMesh);
            }
        }


    } // namespace golias
}