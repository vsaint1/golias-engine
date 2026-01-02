#pragma once

#include "scene/game_object.h"


namespace golias {

    class Mesh;
    class Material;

    class MeshComponent : public Component {
        COMPONENT(MeshComponent)
    public:
        MeshComponent() = default;

        MeshComponent(const std::shared_ptr<Mesh>& pMesh, const std::shared_ptr<Material>& pMaterial);

        virtual ~MeshComponent() = default;

        void Start() override;
        void Update(float deltaTime) override;

        Mesh* GetMesh() const;
        void SetMesh(const std::shared_ptr<Mesh>& pMesh);

        void SetMaterial(const std::shared_ptr<Material>& pMaterial);
        Material* GetMaterial() const;

        void LoadProperties(const nlohmann::json& json);
    private:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
    };

} // namespace golias
