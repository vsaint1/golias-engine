#pragma once

#include "physics/3d/collider.h"
#include <memory>

#include <glm/gtc/quaternion.hpp>

class btRigidBody;

namespace golias {

    /*!
        @brief Type of rigid body.
    */
    enum class EBodyType {
        STATIC, /// Non-movable object. Mass = 0.
        DYNAMIC, /// Movable object affected by forces. Mass > 0.
        KINEMATIC /// Movable object not affected by forces. Mass = 0.
    };


    enum class RigidbodyConstraints {
        NONE              = 0,
        FREEZE_POSITION_X = 1 << 0,
        FREEZE_POSITION_Y = 1 << 1,
        FREEZE_POSITION_Z = 1 << 2,
        FREEZE_ROTATION_X = 1 << 3,
        FREEZE_ROTATION_Y = 1 << 4,
        FREEZE_ROTATION_Z = 1 << 5,
        FREEZE_POSITION   = FREEZE_POSITION_X | FREEZE_POSITION_Y | FREEZE_POSITION_Z,
        FREEZE_ROTATION   = FREEZE_ROTATION_X | FREEZE_ROTATION_Y | FREEZE_ROTATION_Z,
        FREEZE_ALL        = FREEZE_POSITION | FREEZE_ROTATION
    };

    inline RigidbodyConstraints operator|(RigidbodyConstraints a, RigidbodyConstraints b) {
        return static_cast<RigidbodyConstraints>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline bool operator&(RigidbodyConstraints a, RigidbodyConstraints b) {
        return (static_cast<int>(a) & static_cast<int>(b)) != 0;
    }

    class RigidBody {
    public:
        RigidBody(EBodyType bodyType, const std::shared_ptr<Collider>& pCollider, float mass = 1.0f);
        ~RigidBody();

        EBodyType GetBodyType() const;
        btRigidBody* GetRigidBody() const;

        void SetMass(float value);
        float GetMass() const;

        void SetDragging(float value);
        float GetDragging() const;

        void SetAngularDragging(float value);
        float GetAngularDragging() const;

        void SetFriction(float value);
        float GetFriction() const;

        void SetRestitution(float value);
        float GetRestitution() const;

        void SetConstraints(RigidbodyConstraints constraints);
        RigidbodyConstraints GetConstraints() const;

        void SetUseGravity(bool value);
        bool GetUseGravity() const;

        void SetIsKinematic(bool value);
        bool GetIsKinematic() const;

        void SetPosition(const glm::vec3& pos);
        glm::vec3 GetPosition() const;

        void SetRotation(const glm::quat& rot);
        glm::quat GetRotation() const;

        void SetVelocity(const glm::vec3& vel);
        glm::vec3 GetVelocity() const;
        void SetAngularVelocity(const glm::vec3& vel);
        glm::vec3 GetAngularVelocity() const;

        void AddForce(const glm::vec3& force);
        void AddForceAtPosition(const glm::vec3& force, const glm::vec3& position);
        void AddTorque(const glm::vec3& torque);

        void SetAddedToWorld(bool added);
        bool IsAddedToWorld() const;

    private:
        void ApplyConstraints();
        void UpdatePhysicsProperties();

        std::unique_ptr<btRigidBody> rigidBody;
        std::shared_ptr<Collider> collider;

        EBodyType type;

        float _mass                       = 1.0f;
        float drag                       = 0.0f;
        float angularDrag                = 0.05f;
        float friction                   = 0.6f;
        float bounciness                 = 0.0f;
        bool useGravity                  = true;
        RigidbodyConstraints bodyConstraints = RigidbodyConstraints::NONE;

        bool addedToWorld = false;
    };

} // namespace golias
