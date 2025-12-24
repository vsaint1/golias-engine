#pragma once

#include "scene/game_object.h"


namespace golias {

    class Mesh;
    class Material;

    class MeshComponent : public Component {
        COMPONENT(MeshComponent)
    public:
        MeshComponent()          = default;

        MeshComponent(const std::shared_ptr<Mesh>& pMesh, const std::shared_ptr<Material>& pMaterial);

        virtual ~MeshComponent() = default;

        virtual void Update(float deltaTime) override;


    private:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
    };

} // namespace golias
