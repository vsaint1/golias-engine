#include "physics/3d/rigid_body.h"

#include "core/engine.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace golias {

    RigidBody::RigidBody(EBodyType bodyType, const std::shared_ptr<Collider>& pCollider, float mass, float friction)
        : type(bodyType), collider(pCollider), _mass(mass), _friction(friction) {

        btVector3 localInertia(0, 0, 0);

        btScalar btMass = 0.0f;
        if (type == EBodyType::DYNAMIC) {
            btMass = (_mass > 0.0f) ? btScalar(_mass) : btScalar(1.0f); 

            if (collider && collider->GetCollisionShape()) {
                collider->GetCollisionShape()->calculateLocalInertia(btMass, localInertia);
            }
        }

        btTransform transform;
        transform.setIdentity();
        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, motionState, collider->GetCollisionShape(), localInertia);
        rigidBody = std::make_unique<btRigidBody>(rbInfo);
        rigidBody->setFriction(btScalar(_friction));

        if (type == EBodyType::KINEMATIC) {
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
        }

        rigidBody->setMotionState(motionState);
    }


    RigidBody::~RigidBody() {
        if (addedToWorld) {
            Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
        }
    }

    EBodyType RigidBody::GetBodyType() const {
        return type;
    }

    btRigidBody* RigidBody::GetdBody() const {
        return rigidBody.get();
    }

    void RigidBody::SetMass(float value) {
        _mass = value;
    }

    float RigidBody::GetMass() const {
        return _mass;
    }

    void RigidBody::SetFriction(float value) {
        _friction = value;
    }

    float RigidBody::GetFriction() const {
        return _friction;
    }

    void RigidBody::SetAddedToWorld(bool added) {
        addedToWorld = added;
    }

    bool RigidBody::IsAddedToWorld() const {
        return addedToWorld;
    }

    void RigidBody::SetPosition(const glm::vec3& pos) {
        if (!rigidBody) {
            return;
        }

        auto& tr = rigidBody->getWorldTransform();
        tr.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)));

        if (rigidBody->getMotionState()) {
            rigidBody->getMotionState()->setWorldTransform(tr);
        }

        rigidBody->setWorldTransform(tr);
    }

    glm::vec3 RigidBody::GetPosition() const {
        if (!rigidBody) {
            return glm::vec3(0.0f);
        }

        auto& tr         = rigidBody->getWorldTransform();
        btVector3 origin = tr.getOrigin();

        return glm::vec3(origin.getX(), origin.getY(), origin.getZ());
    }

    void RigidBody::SetRotation(const glm::quat& rot) {
        if (!rigidBody) {
            return;
        }

        auto& tr = rigidBody->getWorldTransform();
        tr.setRotation(btQuaternion(btScalar(rot.x), btScalar(rot.y), btScalar(rot.z), btScalar(rot.w)));

        if (rigidBody->getMotionState()) {
            rigidBody->getMotionState()->setWorldTransform(tr);
        }

        rigidBody->setWorldTransform(tr);
    }

    glm::quat RigidBody::GetRotation() const {

        if (!rigidBody) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        const auto& rot = rigidBody->getWorldTransform().getRotation();
        return glm::quat(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
    }


} // namespace golias
