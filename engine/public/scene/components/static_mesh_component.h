#pragma once

#include "component.h"

namespace golias {

    class Mesh;

    class Material;

    class StaticMeshComponent : public Component {

        COMPONENT(StaticMeshComponent)
    public:
        StaticMeshComponent() = default;
        StaticMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material);

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        Ref<Mesh> GetMesh() const;
        void SetMesh(const Ref<Mesh>& mesh);

        Ref<Material> GetMaterial() const;
        void SetMaterial(const Ref<Material>& material);

        bool IsVisible() const;
        void SetVisible(bool visible);

    protected:
        Ref<Mesh> mMesh         = nullptr;
        Ref<Material> mMaterial = nullptr;
        bool mVisible           = true;
    };
} // namespace golias
