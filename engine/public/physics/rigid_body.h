#pragma once
#include "physics/collision.h"
#include "stdafx.h"

class btRigidBody;

namespace golias {

    enum class RigidBodyType {
        Static, /// Represents a static rigid body that does not move and is unaffected by forces.
        Dynamic, /// Represents a dynamic rigid body that can move and is affected by forces.
        Kinematic /// Represents a kinematic rigid body that can be moved manually but is not affected by forces.
    };

    class Collider;


    struct PhysicsMaterial {
        float Friction    = 0.5f; /// The friction coefficient of the rigid body.
        float Mass        = 1.0f; /// The mass of the rigid body.
        float Restitution = 0.0f; /// The restitution (bounciness) of the rigid body.
    };


    /// @brief  Represents a rigid body in the physics simulation, which can be static, dynamic, or kinematic. A rigid body is associated with a collider that defines its shape and collision properties.
    class RigidBody : public CollisionObject {
    public:

        /// @brief  Creates a new RigidBody with the specified type, collider, mass, friction, and restitution.
        /// @param type  The type of the rigid body (Static, Dynamic, Kinematic).
        /// @param collider The collider associated with the rigid body.
        /// @param PhysicsMaterial The physics material defining the mass, friction, and restitution of the rigid body.
        RigidBody(RigidBodyType type, const Ref<Collider>& collider, const PhysicsMaterial& material = PhysicsMaterial());
        ~RigidBody();

        RigidBodyType GetType() const;

        float GetMass() const;
        void SetMass(float mass);

        float GetFriction() const;
        void SetFriction(float friction);

        float GetRestitution() const;
        void SetRestitution(float restitution);

        Ref<Collider> GetCollider() const;

        btRigidBody* GetBody() const;

        bool IsAddedToWorld() const;

        void SetAddedToWorld(bool added);

        glm::vec3 GetPosition() const;
        void SetPosition(const glm::vec3& position);

        glm::quat GetRotation() const;
        void SetRotation(const glm::quat& rotation);

        bool IsEnabled() const;
        void SetEnabled(bool enabled);

        bool IsTrigger() const;
        void SetTrigger(bool isTrigger);

        void ApplyImpulse(const glm::vec3& force);
        void ApplyTorque(const glm::vec3& torque);
        void ApplyForce(const glm::vec3& force);

    private:
        PhysicsMaterial mPhysicsMaterial;

        RigidBodyType mType           = RigidBodyType::Dynamic;
        Ref<Collider> mCollider       = nullptr;
        Scope<btRigidBody> mRigidBody = nullptr;

        int mDefaultCollisionFlags = 0;
        bool mIsAddedToWorld = false;
        bool mIsEnabled      = true;
    };
} // namespace golias
