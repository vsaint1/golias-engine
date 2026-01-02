#include "scene/3d/physics_component.h"

#include "core/engine.h"

namespace golias {

    PhysicsComponent::PhysicsComponent(const std::shared_ptr<RigidBody>& pRigidBody) : rigidBody(pRigidBody) {
    }


    RigidBody* PhysicsComponent::GetRigidBody() const {
        return rigidBody.get();
    }

    void PhysicsComponent::SetRigidBody(std::shared_ptr<RigidBody> pRigidBody) {
        rigidBody = pRigidBody;
    }

    void PhysicsComponent::Start() {
        if (!rigidBody) {
            return;
        }

        const auto& pos = GetOwner()->GetWorldPosition();
        const auto& rot = GetOwner()->GetWorldRotation();

        rigidBody->SetPosition(pos);
        rigidBody->SetRotation(rot);

        Engine::GetInstance().GetPhysicsManager().AddRigidBody(rigidBody.get());
        spdlog::info("PhysicsComponent::Start added RigidBody to PhysicsManager");
    }


    void PhysicsComponent::Update(float deltaTime) {
        if (!rigidBody) {
            return;
        }

        if (rigidBody->GetBodyType() == EBodyType::DYNAMIC) {
            GetOwner()->SetWorldPosition(rigidBody->GetPosition());
            GetOwner()->SetWorldRotation(rigidBody->GetRotation());
        }
    }

    void PhysicsComponent::LoadProperties(const nlohmann::json& json) {
        std::shared_ptr<Collider> collider = nullptr;

        if (json.contains("collider")) {
            auto colliderJson        = json["collider"];
            std::string colliderType = colliderJson.value("type", "box");

            if (colliderType == "box") {
                glm::vec3 extents = glm::vec3(1.0f);

                if (colliderJson.contains("extents")) {
                    auto extentsJson = colliderJson["extents"];
                    extents.x        = extentsJson.value("x", 1.0f);
                    extents.y        = extentsJson.value("y", 1.0f);
                    extents.z        = extentsJson.value("z", 1.0f);
                }

                collider = std::make_shared<BoxCollider>(extents);
            } else if (colliderType == "sphere") {
                float radius = colliderJson.value("radius", 1.0f);
                collider     = std::make_shared<SphereCollider>(radius);
            } else if (colliderType == "capsule") {
                float radius = colliderJson.value("radius", 0.5f);
                float height = colliderJson.value("height", 1.0f);
                collider     = std::make_shared<CapsuleCollider>(radius, height);
            } else {
                spdlog::warn("PhysicsComponent::LoadProperties Unknown collider type: {}", colliderType);
                return;
            }


            if (!collider) {
                spdlog::warn("PhysicsComponent::LoadProperties Failed to create collider of type: {}", colliderType);
                return;
            }

            std::shared_ptr<RigidBody> body = nullptr;

            if (json.contains("rigid_body")) {
                auto bodyJson           = json["rigid_body"];
                std::string bodyTypeStr = bodyJson.value("type", "static");
                EBodyType bodyType      = EBodyType::STATIC;

                if (bodyTypeStr == "dynamic") {
                    bodyType = EBodyType::DYNAMIC;
                } else if (bodyTypeStr == "kinematic") {
                    bodyType = EBodyType::KINEMATIC;
                }

                body = std::make_shared<RigidBody>(bodyType, collider);

                if (bodyJson.contains("mass")) {
                    float mass = bodyJson.value("mass", 1.0f);
                    body->SetMass(mass);
                }

                if (bodyJson.contains("friction")) {
                    float friction = bodyJson.value("friction", 0.6f);
                    body->SetFriction(friction);
                }

                if (bodyJson.contains("restitution")) {
                    float restitution = bodyJson.value("restitution", 0.0f);
                    body->SetRestitution(restitution);
                }

                if (bodyJson.contains("dragging")) {
                    float dragging = bodyJson.value("dragging", 0.0f);
                    body->SetDragging(dragging);
                }

                if (bodyJson.contains("angular_dragging")) {
                    float angularDragging = bodyJson.value("angular_dragging", 0.05f);
                    body->SetAngularDragging(angularDragging);
                }

                if (bodyJson.contains("is_kinematic")) {
                    bool isKinematic = bodyJson.value("is_kinematic", false);
                    body->SetIsKinematic(isKinematic);
                }

                if(body){
                    SetRigidBody(body);
                }
            }
        }
    } // namespace golias
}