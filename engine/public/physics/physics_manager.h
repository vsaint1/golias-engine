#pragma once

#include "stdafx.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;


namespace golias {

    class RigidBody;

    class PhysicsManager {
    public:
        PhysicsManager();
        ~PhysicsManager();

        bool Initialize();

        void Update(float deltaTime);

        void Shutdown();

        btDiscreteDynamicsWorld* GetWorld() const;

        void SetGravity(float x, float y, float z);

        void AddRigidBody(RigidBody* rigidBody);

        void RemoveRigidBody(RigidBody* rigidBody);

    private:
        btBroadphaseInterface* mBroadphase                       = nullptr;
        btDefaultCollisionConfiguration* mCollisionConfiguration = nullptr;
        btCollisionDispatcher* mDispatcher                       = nullptr;
        btSequentialImpulseConstraintSolver* mSolver             = nullptr;
        btDiscreteDynamicsWorld* mWorld                          = nullptr;
    };
} // namespace golias
