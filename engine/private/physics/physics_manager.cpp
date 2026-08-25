#include "physics/physics_manager.h"

#include "physics/rigid_body.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace golias {

    PhysicsManager::PhysicsManager() {
    }

    PhysicsManager::~PhysicsManager() {
        Shutdown();
    }


    bool PhysicsManager::Initialize() {

        mBroadphase             = new btDbvtBroadphase();
        mCollisionConfiguration = new btDefaultCollisionConfiguration();
        mDispatcher             = new btCollisionDispatcher(mCollisionConfiguration);
        mSolver                 = new btSequentialImpulseConstraintSolver();
        mWorld                  = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);

        constexpr btScalar kDefaultGravity = -9.81f;
        mWorld->setGravity(btVector3(0, kDefaultGravity, 0));

        return true;
    }


    void PhysicsManager::Update(float deltaTime) {
        const btScalar kFixedTimeStep = 1.0f / 60.0f;
        const int kMaxSubSteps        = 4;
        mWorld->stepSimulation(deltaTime, kMaxSubSteps, kFixedTimeStep);
    }

    void PhysicsManager::Shutdown() {

        delete mWorld;
        delete mSolver;
        delete mDispatcher;
        delete mCollisionConfiguration;
        delete mBroadphase;

        mWorld                  = nullptr;
        mSolver                 = nullptr;
        mDispatcher             = nullptr;
        mCollisionConfiguration = nullptr;
        mBroadphase             = nullptr;
    }

    btDiscreteDynamicsWorld* PhysicsManager::GetWorld() const {
        return mWorld;
    }

    void PhysicsManager::SetGravity(float x, float y, float z) {
        if (mWorld) {
            mWorld->setGravity(btVector3(x, y, z));
        }
    }

    void PhysicsManager::AddRigidBody(RigidBody* rigidBody) {
        if (rigidBody && mWorld) {
            mWorld->addRigidBody(rigidBody->GetBody(), btBroadphaseProxy::StaticFilter, btBroadphaseProxy::AllFilter);
            rigidBody->SetAddedToWorld(true);
        }
    }

    void PhysicsManager::RemoveRigidBody(RigidBody* rigidBody) {
        if (rigidBody && mWorld) {
            mWorld->removeRigidBody(rigidBody->GetBody());
            rigidBody->SetAddedToWorld(false);
        }
    }

} // namespace golias
