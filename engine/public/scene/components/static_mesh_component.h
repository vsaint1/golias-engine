#pragma once

#include "component.h"
#include "stdafx.h"

namespace golias {

    class Mesh;

    class Material;

    class StaticMeshComponent : public Component {
    public:

        StaticMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material);

        void Update(float deltaTime) override;

        Ref<Mesh> GetMesh() const;
        void SetMesh(const Ref<Mesh>& mesh);

        Ref<Material> GetMaterial() const;
        void SetMaterial(const Ref<Material>& material);

    private:
        Ref<Mesh> mMesh         = nullptr;
        Ref<Material> mMaterial = nullptr;
    };
} // namespace golias
