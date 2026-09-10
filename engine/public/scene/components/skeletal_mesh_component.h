#pragma once

#include "math/aabb.h"
#include "scene/components/static_mesh_component.h"

namespace golias {

    class GameObject;

    struct ModelSkin;

    class SkeletalMeshComponent : public StaticMeshComponent {
        
        COMPONENT_DERIVED(SkeletalMeshComponent, StaticMeshComponent)
    public:
        SkeletalMeshComponent() = default;
        SkeletalMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material);

        bool LoadProperties(const Json& properties) override;
        
        void Start() override;

        void Update(float deltaTime) override;

        /// @brief  Attaches a skin so this mesh is deformed by the joint nodes.
        void SetSkin(const Ref<ModelSkin>& skin);

        /// @brief  Returns the skin attached to this component.
        const Ref<ModelSkin>& GetSkin() const;

    private:
        /// @brief  Caches per-frame joint matrices (inverse of mesh-node world * joint world * inverse bind).
        void UpdateJointMatrices();

    private:
        Ref<ModelSkin> mSkin = nullptr;

        std::vector<GameObject*> mJointObjects;
        std::vector<glm::mat4> mJointMatrices;

        AABB mWorldBounds;
        bool mWorldBoundsValid = false;
        float mBindRadius      = 0.0f;

        bool mJointObjectsResolved = false;
    };
} // namespace golias
