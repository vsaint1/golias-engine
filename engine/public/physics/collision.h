#pragma once
#include "stdafx.h"

namespace golias {

    class ContactListener;

    enum class CollisonObjectType { RigidBody, KinematicCharacterController };

    class CollisionObject;
    class GameObject;
    class PhysicsComponent;
    class PlayerControllerComponent;

    struct Collision {
        GameObject* Object = nullptr;
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Normal   = glm::vec3(0.0f);
    };

    class CollisionObject {

    public:
        CollisonObjectType GetCollisionObjectType();

        GameObject* GetGameObject() const;

        void AddContactListener(ContactListener* listener);
        void RemoveContactListener(ContactListener* listener);

    protected:
        void SetGameObject(GameObject* owner);

    private:
        void DispatchContactEnter(const Collision& collision);
        void DispatchContactExit(const Collision& collision);

    private:
        CollisonObjectType mType = CollisonObjectType::RigidBody;
        GameObject* mOwner       = nullptr;

        std::vector<ContactListener*> mContactListeners = {};

        friend class PhysicsManager;
        friend class PhysicsComponent;
        friend class PlayerControllerComponent;
    };


    class ContactListener {
    public:
        virtual void OnCollisionEnter(const Collision& collision) = 0;
        virtual void OnCollisionExit(const Collision& collision)  = 0;
    };
} // namespace golias
