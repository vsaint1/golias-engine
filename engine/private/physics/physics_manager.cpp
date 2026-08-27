#include "physics/physics_manager.h"

#include "physics/collision.h"
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

        btDispatcher* dispatcher = mWorld->getDispatcher();
        const int manifolds      = dispatcher->getNumManifolds();

        std::unordered_map<ContactPair, ContactState, ContactPairHash> currentContacts;

        for (int i = 0; i < manifolds; ++i) {

            btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);

            if (!manifold) {
                continue;
            }

            CollisionObject* bodyA = static_cast<CollisionObject*>(manifold->getBody0()->getUserPointer());
            CollisionObject* bodyB = static_cast<CollisionObject*>(manifold->getBody1()->getUserPointer());

            if (!bodyA || !bodyB) {
                continue;
            }

            ContactState state;
            bool hasContact    = false;
            const int contacts = manifold->getNumContacts();
            for (int contactIndex = 0; contactIndex < contacts; ++contactIndex) {
                const btManifoldPoint& point = manifold->getContactPoint(contactIndex);
                if (point.getDistance() > 0.0f) {
                    continue;
                }

                const glm::vec3 position(point.m_positionWorldOnB.x(), point.m_positionWorldOnB.y(), point.m_positionWorldOnB.z());
                const glm::vec3 normal(point.m_normalWorldOnB.x(), point.m_normalWorldOnB.y(), point.m_normalWorldOnB.z());
                state.First  = {bodyB->GetGameObject(), position, normal};
                state.Second = {bodyA->GetGameObject(), position, -normal};
                hasContact   = true;
                break;
            }

            if (!hasContact) {
                continue;
            }

            ContactPair pair{bodyA, bodyB};
            if (std::less<CollisionObject*>{}(pair.Second, pair.First)) {
                std::swap(pair.First, pair.Second);
                std::swap(state.First, state.Second);
            }

            currentContacts.emplace(pair, state);
            if (!mContacts.contains(pair)) {
                pair.First->DispatchContactEnter(state.First);
                pair.Second->DispatchContactEnter(state.Second);
            }
        }

        for (const auto& [pair, state] : mContacts) {
            if (!currentContacts.contains(pair)) {
                pair.First->DispatchContactExit(state.First);
                pair.Second->DispatchContactExit(state.Second);
            }
        }

        mContacts = std::move(currentContacts);
    }

    void PhysicsManager::Shutdown() {

        mContacts.clear();

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

            short group = btBroadphaseProxy::StaticFilter;

            if (rigidBody->GetType() == RigidBodyType::Dynamic) {
                group = btBroadphaseProxy::DefaultFilter;
            } else if (rigidBody->GetType() == RigidBodyType::Kinematic) {
                group = btBroadphaseProxy::KinematicFilter;
            }

            mWorld->addRigidBody(rigidBody->GetBody(), group, btBroadphaseProxy::AllFilter);
            rigidBody->SetAddedToWorld(true);
        }
    }

    void PhysicsManager::RemoveRigidBody(RigidBody* rigidBody) {
        if (rigidBody && mWorld) {
            mWorld->removeRigidBody(rigidBody->GetBody());
            rigidBody->SetAddedToWorld(false);
            ForgetCollisionObject(rigidBody);
        }
    }

    void PhysicsManager::ForgetCollisionObject(CollisionObject* object) {
        if (!object) {
            return;
        }

        for (auto it = mContacts.begin(); it != mContacts.end();) {
            if (it->first.First == object || it->first.Second == object) {
                it = mContacts.erase(it);
            } else {
                ++it;
            }
        }
    }

} // namespace golias
