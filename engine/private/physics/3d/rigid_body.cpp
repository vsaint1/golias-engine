#include "core/engine.h"
#include "physics/3d/rigid_body.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace golias {

    RigidBody::RigidBody(EBodyType type, const std::shared_ptr<Collider>& pCollider, float _mass, float _friciton)
        : body_type(type), collider(pCollider), mass(_mass), friction(_friciton) {

        btVector3 localInertia(0, 0, 0);

        if (type == EBodyType::DYNAMIC && mass != 0.0f && collider && collider->GetCollisionShape()) {
            collider->GetCollisionShape()->calculateLocalInertia(btScalar(mass), localInertia);
        }

        btTransform transform;
        transform.setIdentity();

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        btScalar btMass = type == EBodyType::DYNAMIC ? btScalar(mass) : btScalar(0.0f);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, motionState, collider->GetCollisionShape(), localInertia);

        rigid_body = std::make_unique<btRigidBody>(rbInfo);

        rigid_body->setFriction(btScalar(friction));

        if (type == EBodyType::KINEMATIC) {
            rigid_body->setCollisionFlags(rigid_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigid_body->setActivationState(DISABLE_DEACTIVATION);
        }


        rigid_body->setMotionState(motionState);
    }


    RigidBody::~RigidBody() {
        if (added_to_world) {
            Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
        }
    }

    EBodyType RigidBody::GetBodyType() const {
        return body_type;
    }

    btRigidBody* RigidBody::GetdBody() const {
        return rigid_body.get();
    }

    void RigidBody::SetMass(float value) {
        mass = value;
    }

    float RigidBody::GetMass() const {
        return mass;
    }

    void RigidBody::SetFriction(float value) {
        friction = value;
    }

    float RigidBody::GetFriction() const {
        return friction;
    }

    void RigidBody::SetAddedToWorld(bool added) {
        added_to_world = added;
    }

    bool RigidBody::IsAddedToWorld() const {
        return added_to_world;
    }

    void RigidBody::SetPosition(const glm::vec3& pos) {
        if (!rigid_body) {
            return;
        }

        auto& tr = rigid_body->getWorldTransform();
        tr.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)));

        if (rigid_body->getMotionState()) {
            rigid_body->getMotionState()->setWorldTransform(tr);
        }

        rigid_body->setWorldTransform(tr);
    }

    glm::vec3 RigidBody::GetPosition() const {
        if (!rigid_body) {
            return glm::vec3(0.0f);
        }

        auto& tr         = rigid_body->getWorldTransform();
        btVector3 origin = tr.getOrigin();

        return glm::vec3(static_cast<float>(origin.getX()), static_cast<float>(origin.getY()), static_cast<float>(origin.getZ()));
    }

    void RigidBody::SetRotation(const glm::quat& rot) {
        if (!rigid_body) {
            return;
        }

        auto& tr = rigid_body->getWorldTransform();
        tr.setRotation(btQuaternion(btScalar(rot.x), btScalar(rot.y), btScalar(rot.z), btScalar(rot.w)));

        if (rigid_body->getMotionState()) {
            rigid_body->getMotionState()->setWorldTransform(tr);
        }

        rigid_body->setWorldTransform(tr);
    }

    glm::quat RigidBody::GetRotation() const {

        if (!rigid_body) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        const auto& rot = rigid_body->getWorldTransform().getRotation();
        return glm::quat(
            static_cast<float>(rot.getW()), static_cast<float>(rot.getX()), static_cast<float>(rot.getY()), static_cast<float>(rot.getZ()));
    }

    void RigidBody::SetScale(const glm::vec3& scl) {
        scale = scl;
    }

    glm::vec3 RigidBody::GetScale() const {
        return scale;
    }
} // namespace golias
