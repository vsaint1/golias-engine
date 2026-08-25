#include "physics/rigid_body.h"

#include "core/engine.h"
#include "physics/collider.h"
#include <btBulletDynamicsCommon.h>

namespace golias {

    RigidBody::RigidBody(RigidBodyType type, const Ref<Collider>& collider, const PhysicsMaterial& material)
        : mType(type), mCollider(collider), mPhysicsMaterial(material) {

        if (!collider) {
            GOLIAS_LOG_ERROR("Cannot create RigidBody without a valid Collider.");
            return;
        }

        btVector3 inertia(0, 0, 0);

        if (mType == RigidBodyType::Dynamic && mPhysicsMaterial.Mass > 0.0f && mCollider->GetShape()) {
            mCollider->GetShape()->calculateLocalInertia(mPhysicsMaterial.Mass, inertia);
        }

        btTransform transform;
        transform.setIdentity();

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        btScalar btMass = (mType == RigidBodyType::Dynamic) ? mPhysicsMaterial.Mass : 0.0f;

        btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, motionState, mCollider->GetShape(), inertia);


        mRigidBody = std::make_unique<btRigidBody>(rbInfo);

        mRigidBody->setFriction(mPhysicsMaterial.Friction);
        mRigidBody->setRestitution(mPhysicsMaterial.Restitution);

        if (mType == RigidBodyType::Kinematic) {
            mRigidBody->setCollisionFlags(mRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            mRigidBody->setActivationState(DISABLE_DEACTIVATION);
        }
    }

    RigidBody::~RigidBody() {
        Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
    }

    RigidBodyType RigidBody::GetType() const {
        return mType;
    }

    Ref<Collider> RigidBody::GetCollider() const {
        return mCollider;
    }

    btRigidBody* RigidBody::GetBody() const {
        return mRigidBody.get();
    }

    bool RigidBody::IsAddedToWorld() const {
        return mIsAddedToWorld;
    }

    void RigidBody::SetAddedToWorld(bool added) {
        mIsAddedToWorld = added;
    }

    float RigidBody::GetMass() const {
        return mPhysicsMaterial.Mass;
    }

    void RigidBody::SetMass(float mass) {
        mPhysicsMaterial.Mass = mass;
    }

    float RigidBody::GetFriction() const {
        return mPhysicsMaterial.Friction;
    }

    void RigidBody::SetFriction(float friction) {
        mPhysicsMaterial.Friction = friction;
    }

    float RigidBody::GetRestitution() const {
        return mPhysicsMaterial.Restitution;
    }

    void RigidBody::SetRestitution(float restitution) {
        mPhysicsMaterial.Restitution = restitution;
    }

    glm::vec3 RigidBody::GetPosition() const {

        btVector3 origin = mRigidBody->getWorldTransform().getOrigin();

        glm::vec3 pos = glm::vec3(origin.x(), origin.y(), origin.z());

        return pos;
    }

    void RigidBody::SetPosition(const glm::vec3& position) {
        if (!mRigidBody) {
            return;
        }

        btTransform transform = mRigidBody->getWorldTransform();
        transform.setOrigin(btVector3(position.x, position.y, position.z));

        if (btMotionState* motionState = mRigidBody->getMotionState()) {
            motionState->setWorldTransform(transform);
        }

        mRigidBody->setWorldTransform(transform);
    }

    glm::quat RigidBody::GetRotation() const {

        btTransform transform = mRigidBody->getWorldTransform();
        btQuaternion rotation = transform.getRotation();
        return glm::quat(rotation.w(), rotation.x(), rotation.y(), rotation.z());
    }

    void RigidBody::SetRotation(const glm::quat& rotation) {
        if (!mRigidBody) {
            return;
        }

        btTransform transform = mRigidBody->getWorldTransform();
        transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        mRigidBody->setWorldTransform(transform);
    }
} // namespace golias
