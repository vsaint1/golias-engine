#include "physics/3d/rigid_body.h"

#include "core/engine.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace golias {
    RigidBody::RigidBody(EBodyType bodyType, const std::shared_ptr<Collider>& pCollider, float mass)
        : type(bodyType), collider(pCollider), _mass(mass) {

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

        if (type == EBodyType::KINEMATIC) {
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
            useGravity = false;
        } else if (type == EBodyType::STATIC) {
            useGravity = false;
        } else if (type == EBodyType::DYNAMIC) {
            rigidBody->setActivationState(ACTIVE_TAG);
        }

        UpdatePhysicsProperties();
    }

    RigidBody::~RigidBody() {
        if (addedToWorld) {
            Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
        }
    }

    void RigidBody::UpdatePhysicsProperties() {
        if (!rigidBody) {
            return;
        }

        rigidBody->setFriction(btScalar(friction));
        rigidBody->setRestitution(btScalar(bounciness));

        rigidBody->setDamping(btScalar(drag), btScalar(angularDrag));

        if (type == EBodyType::DYNAMIC) {
            if (useGravity) {
                auto world = Engine::GetInstance().GetPhysicsManager().GetPhyisicsWorld();
                if (world) {
                    rigidBody->setGravity(world->getGravity());
                }
            } else {
                rigidBody->setGravity(btVector3(0, 0, 0));
            }
        }

        ApplyConstraints();
    }

    void RigidBody::ApplyConstraints() {
        if (!rigidBody) {
            return;
        }

        btVector3 linearFactor(1, 1, 1);
        if (bodyConstraints & RigidbodyConstraints::FREEZE_POSITION_X) {
            linearFactor.setX(0);
        }

        if (bodyConstraints & RigidbodyConstraints::FREEZE_POSITION_Y) {
            linearFactor.setY(0);
        }

        if (bodyConstraints & RigidbodyConstraints::FREEZE_POSITION_Z) {
            linearFactor.setZ(0);
        }

        rigidBody->setLinearFactor(linearFactor);

        btVector3 angularFactor(1, 1, 1);
        if (bodyConstraints & RigidbodyConstraints::FREEZE_ROTATION_X) {
            angularFactor.setX(0);
        }

        if (bodyConstraints & RigidbodyConstraints::FREEZE_ROTATION_Y) {
            angularFactor.setY(0);
        }

        if (bodyConstraints & RigidbodyConstraints::FREEZE_ROTATION_Z) {
            angularFactor.setZ(0);
        }

        rigidBody->setAngularFactor(angularFactor);
    }


    EBodyType RigidBody::GetBodyType() const {
        return type;
    }

    btRigidBody* RigidBody::GetRigidBody() const {
        return rigidBody.get();
    }

    void RigidBody::SetMass(float value) {
        if (type != EBodyType::DYNAMIC) {
            return;
        }

        _mass = value;
        btVector3 inertia(0, 0, 0);
        if (collider && collider->GetCollisionShape()) {
            collider->GetCollisionShape()->calculateLocalInertia(btScalar(_mass), inertia);
        }
        rigidBody->setMassProps(btScalar(_mass), inertia);
        rigidBody->updateInertiaTensor();
    }

    void RigidBody::SetDragging(float value) {
        drag = value;
        rigidBody->setDamping(btScalar(drag), btScalar(angularDrag));
    }

    void RigidBody::SetAngularDragging(float value) {
        angularDrag = value;
        rigidBody->setDamping(btScalar(drag), btScalar(angularDrag));
    }

    bool RigidBody::GetUseGravity() const {
        return useGravity;
    }

    bool RigidBody::GetIsKinematic() const {
        return type == EBodyType::KINEMATIC;
    }

    RigidbodyConstraints RigidBody::GetConstraints() const {
        return bodyConstraints;
    }

    float RigidBody::GetRestitution() const {
        return bounciness;
    }

    float RigidBody::GetFriction() const {
        return friction;
    }

    float RigidBody::GetDragging() const {
        return drag;
    }

    float RigidBody::GetMass() const {
        return _mass;
    }
    
    float RigidBody::GetAngularDragging() const {
        return angularDrag;
    }

    void RigidBody::SetFriction(float value) {
        friction = value;
        rigidBody->setFriction(btScalar(friction));
    }

    void RigidBody::SetRestitution(float value) {
        bounciness = value;
        rigidBody->setRestitution(btScalar(bounciness));
    }

    void RigidBody::SetConstraints(RigidbodyConstraints constraints) {
        bodyConstraints = constraints;
        ApplyConstraints();
    }

    void RigidBody::SetUseGravity(bool value) {
        useGravity = value;
        UpdatePhysicsProperties();
    }

    void RigidBody::SetIsKinematic(bool value) {
        if (value && type != EBodyType::KINEMATIC) {
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigidBody->setActivationState(DISABLE_DEACTIVATION);

            btVector3 inertia(0, 0, 0);
            rigidBody->setMassProps(0.0f, inertia);
            rigidBody->updateInertiaTensor();

            type        = EBodyType::KINEMATIC;
            useGravity = false;
        } else if (!value && type == EBodyType::KINEMATIC) {
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
            type        = EBodyType::DYNAMIC;
            useGravity = true;

            SetMass(_mass);
        }

        UpdatePhysicsProperties();
    }


    void RigidBody::SetPosition(const glm::vec3& pos) {
        if (!rigidBody) {
            return;
        }

        btTransform& tr = rigidBody->getWorldTransform();
        tr.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)));

        if (rigidBody->getMotionState()) {
            rigidBody->getMotionState()->setWorldTransform(tr);
        }
        rigidBody->setWorldTransform(tr);

        // rigidBody->activate(true);
    }

    glm::vec3 RigidBody::GetPosition() const {
        if (!rigidBody) {
            return glm::vec3(0.0f);
        }

        const btTransform& tr   = rigidBody->getWorldTransform();
        const btVector3& origin = tr.getOrigin();
        return glm::vec3(origin.getX(), origin.getY(), origin.getZ());
    }

    void RigidBody::SetRotation(const glm::quat& rot) {
        if (!rigidBody) {
            return;
        }

        btTransform& tr = rigidBody->getWorldTransform();
        tr.setRotation(btQuaternion(btScalar(rot.x), btScalar(rot.y), btScalar(rot.z), btScalar(rot.w)));

        if (rigidBody->getMotionState()) {
            rigidBody->getMotionState()->setWorldTransform(tr);
        }
        rigidBody->setWorldTransform(tr);

        // rigidBody->activate(true);
    }

    glm::quat RigidBody::GetRotation() const {
        if (!rigidBody) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        const btQuaternion& rot = rigidBody->getWorldTransform().getRotation();
        return glm::quat(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
    }


    void RigidBody::SetVelocity(const glm::vec3& vel) {
        if (!rigidBody || type != EBodyType::DYNAMIC) {
            return;
        }
        rigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
        rigidBody->activate(true);
    }

    glm::vec3 RigidBody::GetVelocity() const {
        if (!rigidBody) {
            return glm::vec3(0.0f);
        }
        const btVector3& vel = rigidBody->getLinearVelocity();
        return glm::vec3(vel.x(), vel.y(), vel.z());
    }

    void RigidBody::SetAngularVelocity(const glm::vec3& vel) {
        if (!rigidBody || type != EBodyType::DYNAMIC) {
            return;
        }
        rigidBody->setAngularVelocity(btVector3(vel.x, vel.y, vel.z));
        rigidBody->activate(true);
    }

    glm::vec3 RigidBody::GetAngularVelocity() const {
        if (!rigidBody) {
            return glm::vec3(0.0f);
        }
        const btVector3& vel = rigidBody->getAngularVelocity();
        return glm::vec3(vel.x(), vel.y(), vel.z());
    }


    void RigidBody::AddForce(const glm::vec3& force) {
        if (!rigidBody || type != EBodyType::DYNAMIC) {
            return;
        }
        rigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
        rigidBody->activate(true);
    }

    void RigidBody::AddForceAtPosition(const glm::vec3& force, const glm::vec3& position) {
        if (!rigidBody || type != EBodyType::DYNAMIC) {
            return;
        }
        btVector3 relPos = btVector3(position.x, position.y, position.z) - rigidBody->getCenterOfMassPosition();
        rigidBody->applyForce(btVector3(force.x, force.y, force.z), relPos);
        rigidBody->activate(true);
    }

    void RigidBody::AddTorque(const glm::vec3& torque) {
        if (!rigidBody || type != EBodyType::DYNAMIC) {
            return;
        }

        rigidBody->applyTorque(btVector3(torque.x, torque.y, torque.z));
        rigidBody->activate(true);
    }

    void RigidBody::SetAddedToWorld(bool added) {
        addedToWorld = added;
    }

    bool RigidBody::IsAddedToWorld() const {
        return addedToWorld;
    }


} // namespace golias
