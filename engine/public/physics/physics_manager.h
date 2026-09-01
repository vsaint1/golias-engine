#pragma once

#include "physics/collision.h"
#include "stdafx.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;


namespace golias {

    class RigidBody;

    struct ContactPair {
        CollisionObject* First  = nullptr;
        CollisionObject* Second = nullptr;

        bool operator==(const ContactPair& other) const {
            return First == other.First && Second == other.Second;
        }
    };

    constexpr size_t kMagicContactPairHash = 0x9e3779b9;

    struct ContactPairHash {
        size_t operator()(const ContactPair& pair) const {
            const size_t first  = std::hash<CollisionObject*>{}(pair.First);
            const size_t second = std::hash<CollisionObject*>{}(pair.Second);
            return first ^ (second + kMagicContactPairHash + (first << 6) + (first >> 2));
        }
    };


    class PhysicsManager {
    public:
        static constexpr float kFixedTimeStep = 1.0f / 60.0f;
        static constexpr int kMaxSubSteps        = 4;

        PhysicsManager();
        ~PhysicsManager();

        bool Initialize();

        void Update(float deltaTime);

        void Shutdown();

        btDiscreteDynamicsWorld* GetWorld() const;

        void SetGravity(float x, float y, float z);

        void AddRigidBody(RigidBody* rigidBody);

        void RemoveRigidBody(RigidBody* rigidBody);

        void ForgetCollisionObject(CollisionObject* object);

        float GetFixedTimeStep() const;

        int GetMaxSubSteps() const;

    private:
        struct ContactState {
            Collision First;
            Collision Second;
        };

        btBroadphaseInterface* mBroadphase                       = nullptr;
        btDefaultCollisionConfiguration* mCollisionConfiguration = nullptr;
        btCollisionDispatcher* mDispatcher                       = nullptr;
        btSequentialImpulseConstraintSolver* mSolver             = nullptr;
        btDiscreteDynamicsWorld* mWorld                          = nullptr;
        std::unordered_map<ContactPair, ContactState, ContactPairHash> mContacts;
    };
} // namespace golias
