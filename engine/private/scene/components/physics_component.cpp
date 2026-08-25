#include "scene/components/physics_component.h"

#include "core/engine.h"
#include "physics/rigid_body.h"
#include "scene/game_object.h"

namespace golias {

    PhysicsComponent::PhysicsComponent(const Ref<RigidBody>& rigidBody) : mRigidBody(rigidBody) {
    }

    void PhysicsComponent::Start() {
        if (!mRigidBody) {
            GOLIAS_LOG_ERROR("PhysicsComponent requires a valid RigidBody to function properly.");
            return;
        }

        const glm::vec3 position = GetOwner()->GetWorldPosition();
        const glm::quat rotation = GetOwner()->GetRotation();

        mRigidBody->SetPosition(position);
        mRigidBody->SetRotation(rotation);

        Engine::GetInstance().GetPhysicsManager().AddRigidBody(mRigidBody.get());
    }

    void PhysicsComponent::Update(float deltaTime) {
        if (mRigidBody && mRigidBody->IsAddedToWorld() && mRigidBody->GetType() == RigidBodyType::Dynamic) {
            glm::vec3 position = mRigidBody->GetPosition();
            glm::quat rotation = mRigidBody->GetRotation();

            GetOwner()->SetWorldPosition(position);
            GetOwner()->SetWorldRotation(rotation);
        }
    }

} // namespace golias
