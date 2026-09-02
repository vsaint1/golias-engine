#include "scene/components/physics_component.h"

#include "core/engine.h"
#include "physics/collider.h"
#include "physics/rigid_body.h"
#include "scene/game_object.h"

namespace golias {

    PhysicsComponent::PhysicsComponent(const Ref<RigidBody>& rigidBody) : mRigidBody(rigidBody) {
    }

    PhysicsComponent::~PhysicsComponent() {
        if (mRigidBody && mStarted) {
            mRigidBody->RemoveContactListener(this);
        }
    }

    bool PhysicsComponent::LoadProperties(const Json& properties) {
        if (properties.contains("body") && properties["body"].is_object()) {
            const auto& rigidBodyObj = properties["body"];

            RigidBodyType type = RigidBodyType::Dynamic;
            if (rigidBodyObj.contains("type")) {
                String typeStr = rigidBodyObj["type"].get<String>();

                if (typeStr == "static") {
                    type = RigidBodyType::Static;
                } else if (typeStr == "dynamic") {
                    type = RigidBodyType::Dynamic;
                } else if (typeStr == "kinematic") {
                    type = RigidBodyType::Kinematic;
                } else {
                    GOLIAS_LOG_ERROR("Invalid RigidBody type '%s' in JSON.", typeStr.data());
                    return false;
                }
            }

            Ref<Collider> collider = nullptr;
            if (rigidBodyObj.contains("collider") && rigidBodyObj["collider"].is_object()) {
                const auto& colliderObj = rigidBodyObj["collider"];

                String type = colliderObj.value("type", "");

                if (type == "box") {
                    glm::vec3 size = glm::vec3(1.0f);

                    if (colliderObj.contains("size")) {
                        const auto& sizeObj = colliderObj["size"];
                        size.x              = sizeObj.value("x", 1.0f);
                        size.y              = sizeObj.value("y", 1.0f);
                        size.z              = sizeObj.value("z", 1.0f);
                    }

                    collider = std::make_shared<BoxCollider>(size);
                } else if (type == "sphere") {
                    float radius = colliderObj.value("radius", 1.0f);
                    collider     = std::make_shared<SphereCollider>(radius);
                } else if (type == "capsule") {
                    float radius = colliderObj.value("radius", 1.0f);
                    float height = colliderObj.value("height", 1.0f);
                    collider     = std::make_shared<CapsuleCollider>(radius, height);
                } else {
                    GOLIAS_LOG_ERROR("Unsupported collider type '%s' in JSON.", type.data());
                    return false;
                }

                if (colliderObj.contains("is_trigger")) {
                    collider->SetTrigger(colliderObj["is_trigger"].get<bool>());
                }
            }

            if (!collider) {
                GOLIAS_LOG_ERROR("Collider is required for RigidBody.");
                return false;
            }

            PhysicsMaterial material;
            if (rigidBodyObj.contains("physics_material") && rigidBodyObj["physics_material"].is_object()) {
                const auto& materialObj = rigidBodyObj["physics_material"];
                material.Mass           = materialObj.value("mass", 1.0f);
                material.Friction       = materialObj.value("friction", 0.5f);
                material.Restitution    = materialObj.value("restitution", 0.0f);
            }

            mRigidBody = std::make_shared<RigidBody>(type, collider, material);
            return true;
        }

        GOLIAS_LOG_ERROR("Missing or invalid 'body' property in JSON.");
        return false;
    }
    
    void PhysicsComponent::Start() {
        if (!mRigidBody) {
            GOLIAS_LOG_ERROR("PhysicsComponent requires a valid RigidBody to function properly.");
            return;
        }

        const glm::vec3 position = GetOwner()->GetWorldPosition();
        const glm::quat rotation = GetOwner()->GetRotation();

        mRigidBody->SetGameObject(GetOwner());
        mRigidBody->SetPosition(position);
        mRigidBody->SetRotation(rotation);

        Engine::GetInstance().GetPhysicsManager().AddRigidBody(mRigidBody.get());
        mRigidBody->AddContactListener(this);
        mStarted = true;
    }

    void PhysicsComponent::Update(float deltaTime) {
        if (mRigidBody && mRigidBody->IsAddedToWorld() && mRigidBody->GetType() == RigidBodyType::Dynamic) {
            glm::vec3 position = mRigidBody->GetPosition();
            glm::quat rotation = mRigidBody->GetRotation();

            GetOwner()->SetWorldPosition(position);
            GetOwner()->SetWorldRotation(rotation);
        }
    }

    RigidBody* PhysicsComponent::GetRigidBody() const {
        return mRigidBody.get();
    }

    void PhysicsComponent::SetRigidBody(const Ref<RigidBody>& rigidBody) {
        if (mRigidBody && mStarted) {
            mRigidBody->RemoveContactListener(this);
        }

        mRigidBody = rigidBody;

        if (mRigidBody && mStarted) {
            mRigidBody->AddContactListener(this);
        }
    }

    void PhysicsComponent::OnCollisionEnter(const Collision& collision) {
        if (GetOwner()) {
            GetOwner()->OnCollisionEnter(collision);
        }
    }

    void PhysicsComponent::OnCollisionExit(const Collision& collision) {
        if (GetOwner()) {
            GetOwner()->OnCollisionExit(collision);
        }
    }

} // namespace golias
