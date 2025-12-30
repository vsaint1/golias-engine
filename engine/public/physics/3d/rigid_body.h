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


    class RigidBody {
    public:
        RigidBody(EBodyType type, const std::shared_ptr<Collider>& pCollider, float _mass = 0.0f, float _friction = 0.5f);
        ~RigidBody();

        EBodyType GetBodyType() const;

        btRigidBody* GetdBody() const;

        void SetMass(float value);
        float GetMass() const;

        void SetFriction(float value);
        float GetFriction() const;

        void SetAddedToWorld(bool added);
        bool IsAddedToWorld() const;

        void SetPosition(const glm::vec3& pos);
        glm::vec3 GetPosition() const;
        void SetRotation(const glm::quat& rot);
        glm::quat GetRotation() const;
        void SetScale(const glm::vec3& scl);
        glm::vec3 GetScale() const;

    private:
        EBodyType body_type = EBodyType::STATIC;
        std::shared_ptr<Collider> collider;
        float mass                              = 0.0f;
        float friction                          = 0.5f;
        bool added_to_world                     = false;
        std::unique_ptr<btRigidBody> rigid_body = nullptr;

        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale    = glm::vec3(1.0f);
    };
} // namespace golias
