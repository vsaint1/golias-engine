#pragma once

#include "scene/game_object.h"


namespace golias {

    class Mesh;
    class Material;

    class MeshRendererComponent : public Component {
        COMPONENT(MeshRendererComponent)
    public:
        MeshRendererComponent() = default;

        MeshRendererComponent(const std::shared_ptr<Mesh>& pMesh, const std::shared_ptr<Material>& pMaterial);

        virtual ~MeshRendererComponent() = default;

        void Start() override;
        void Update(float deltaTime) override;

        Mesh* GetMesh() const;
        void SetMesh(const std::shared_ptr<Mesh>& pMesh);

        void SetMaterial(const std::shared_ptr<Material>& pMaterial);
        Material* GetMaterial() const;

        void LoadProperties(const nlohmann::json& json) override;
    private:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
    };

} // namespace golias
