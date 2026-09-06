#include "scene/components/skeletal_mesh_component.h"

#include "core/engine.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/model.h"
#include "scene/game_object.h"

namespace golias {

    SkeletalMeshComponent::SkeletalMeshComponent(const Ref<Mesh>& mesh, const Ref<Material>& material)
        : StaticMeshComponent(mesh, material) {
    }

    void SkeletalMeshComponent::SetSkin(const Ref<ModelSkin>& skin) {
        mSkin                 = skin;
        mJointObjectsResolved = false;
    }

    const Ref<ModelSkin>& SkeletalMeshComponent::GetSkin() const {
        return mSkin;
    }

    void SkeletalMeshComponent::UpdateJointMatrices() {
        if (!mSkin || mSkin->jointNames.empty()) {
            return;
        }

        if (!mJointObjectsResolved) {
            mJointObjects.clear();

            // Scope the joint lookup to THIS instance.
            GameObject* instanceRoot = GetOwner();
            while (instanceRoot->GetParent() && instanceRoot->GetParent()->GetParent()) {
                instanceRoot = instanceRoot->GetParent();
            }

            for (const String& jointName : mSkin->jointNames) {
                mJointObjects.push_back(instanceRoot ? instanceRoot->FindChildByName(jointName) : nullptr);
            }

            mJointObjectsResolved = true;
        }

        const GameObject* parentObject = GetOwner()->GetParent();

        const glm::mat4 meshNodeWorldInverse =
            glm::inverse(parentObject ? parentObject->GetWorldTransform() : GetOwner()->GetWorldTransform());

        const size_t jointCount = std::min(mSkin->jointNames.size(), mSkin->inverseBindMatrices.size());
        mJointMatrices.resize(jointCount);

        for (size_t i = 0; i < jointCount; ++i) {
            GameObject* jointObject = mJointObjects[i];
            if (!jointObject) {
                mJointMatrices[i] = glm::mat4(1.0f);
                continue;
            }

            mJointMatrices[i] = meshNodeWorldInverse * jointObject->GetWorldTransform() * mSkin->inverseBindMatrices[i];
        }
    }

    // TODO: Load skin from JSON (or prefab) if specified???
    bool SkeletalMeshComponent::LoadProperties(const Json& properties) {
        StaticMeshComponent::LoadProperties(properties);

        return true;
    }

    void SkeletalMeshComponent::Start() {
        StaticMeshComponent::Start();

       
    }

    void SkeletalMeshComponent::Update(float deltaTime) {

        if (!mMaterial || !mMesh || !mVisible) {
            return;
        }

        RenderCommand command;
        command.Mesh     = mMesh.get();
        command.Material = mMaterial.get();
        command.Model    = GetOwner()->GetWorldTransform();

        if (mSkin) {
            UpdateJointMatrices();
            command.JointMatrices = mJointMatrices.empty() ? nullptr : mJointMatrices.data();
            command.JointCount    = static_cast<uint32_t>(mJointMatrices.size());
        }

        Engine::GetInstance().GetCommandQueue().Submit(command);
    }

} // namespace golias
