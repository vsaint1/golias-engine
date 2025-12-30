#include "scene/3d/physics_component.h"

#include "core/engine.h"

namespace golias {

    PhysicsComponent::PhysicsComponent(const std::shared_ptr<RigidBody>& pRigidBody) : rigid_body(pRigidBody) {
    }


    RigidBody* PhysicsComponent::GetRigidBody() const {
        return rigid_body.get();
    }

    void PhysicsComponent::Start() {
        if (!rigid_body) {
            return;
        }

        const auto pos = GetOwner()->GetWorldPosition();
        const auto rot = GetOwner()->GetRotation();

        rigid_body->SetPosition(pos);
        rigid_body->SetRotation(rot);

        Engine::GetInstance().GetPhysicsManager().AddRigidBody(rigid_body.get());
        spdlog::info("PhysicsComponent::Start added RigidBody to PhysicsManager");
    }


    void PhysicsComponent::Update(float deltaTime) {
        if (!rigid_body) {
            return;
        }

        if (rigid_body->GetBodyType() == EBodyType::DYNAMIC) {
            GetOwner()->SetPosition(rigid_body->GetPosition());
            GetOwner()->SetRotation(rigid_body->GetRotation());
        }else if (rigid_body->GetBodyType() == EBodyType::KINEMATIC) {
            const auto pos = GetOwner()->GetWorldPosition();
            const auto rot = GetOwner()->GetRotation();

            rigid_body->SetPosition(pos);
            rigid_body->SetRotation(rot);
        }
    }

} // namespace golias
