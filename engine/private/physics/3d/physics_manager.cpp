#include "physics/3d/physics_manager.h"

#include "core/engine.h"
#include "physics/3d/collision_info.h"
#include "physics/3d/rigid_body.h"
#include "scene/game_object.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


namespace golias {

    PhysicsManager::PhysicsManager() = default;

    PhysicsManager::~PhysicsManager() {

        if (dynamicsWorld) {
            int numCollisionObjects = dynamicsWorld->getNumCollisionObjects();
            for (int i = numCollisionObjects - 1; i >= 0; --i) {
                btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
                btRigidBody* body      = btRigidBody::upcast(obj);
                if (body) {
                    dynamicsWorld->removeRigidBody(body);
                }
            }
        }

        dynamicsWorld.reset();
        solver.reset();
        dispatcher.reset();
        broadphase.reset();
        collisionConfiguration.reset();
    }

    bool PhysicsManager::Initialize(PhysicsDebugDrawer* pDebugDrawer) {

        broadphase = std::make_unique<btDbvtBroadphase>();

        collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());

        solver = std::make_unique<btSequentialImpulseConstraintSolver>();

        dynamicsWorld =
            std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get());

        if (!dynamicsWorld) {
            return false;
        }

        constexpr float DEFAULT_GRAVITY = -9.81f;
        dynamicsWorld->setGravity(btVector3(0, DEFAULT_GRAVITY, 0));

        debugDrawer = pDebugDrawer;

        if (debugDrawer) {
            debugDrawer->Initialize();
            dynamicsWorld->setDebugDrawer(debugDrawer);
            debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints);
            SetDebugDrawEnabled(true);
        }

        return true;
    }

    void PhysicsManager::StepSimulation(float deltaTime) {
        if (!dynamicsWorld) {
            return;
        }

        // NOTE: this needs to be fixed, currently using 60 FPS sometimes causes issues with the simulation
        const float MIN_DELTA = 0.0001f;
        const float MAX_DELTA = 0.1f;
        deltaTime             = glm::clamp(deltaTime, MIN_DELTA, MAX_DELTA);

        constexpr btScalar FIXED_TIME_STEP = 1.0f / 60.0f;

        constexpr int MAX_SUB_STEPS = 4;

        dynamicsWorld->stepSimulation(deltaTime, MAX_SUB_STEPS, FIXED_TIME_STEP);

        ProcessCollisionCallbacks();
    }

    void PhysicsManager::ProcessCollisionCallbacks() {
        if (!dynamicsWorld || !dispatcher) {
            return;
        }

        std::set<CollisionPair> currentContacts;

        int numManifolds = dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; ++i) {
            btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);
            if (!manifold || manifold->getNumContacts() == 0) {
                continue;
            }

            const btCollisionObject* objA = manifold->getBody0();
            const btCollisionObject* objB = manifold->getBody1();
            if (!objA || !objB) continue;

            auto* goA = static_cast<GameObject*>(objA->getUserPointer());
            auto* goB = static_cast<GameObject*>(objB->getUserPointer());
            if (!goA || !goB) continue;
            if (!goA->IsAlive() || !goB->IsAlive()) continue;

            CollisionPair pair = (goA < goB) ? CollisionPair{goA, goB} : CollisionPair{goB, goA};
            currentContacts.insert(pair);

            const btRigidBody* btBodyA = btRigidBody::upcast(objA);
            const btRigidBody* btBodyB = btRigidBody::upcast(objB);

            glm::vec3 relVel(0.0f);
            if (btBodyA && btBodyB) {
                btVector3 rv = btBodyA->getLinearVelocity() - btBodyB->getLinearVelocity();
                relVel = glm::vec3(rv.x(), rv.y(), rv.z());
            }

            std::vector<ContactPoint> contactsAtoB;
            for (int j = 0; j < manifold->getNumContacts(); ++j) {
                const btManifoldPoint& pt = manifold->getContactPoint(j);
                if (pt.getDistance() > 0.02f) continue; 

                ContactPoint cp;
                btVector3 pos = pt.getPositionWorldOnB();
                btVector3 nrm = pt.m_normalWorldOnB;

                cp.point    = glm::vec3(pos.x(), pos.y(), pos.z());
                cp.normal   = glm::vec3(nrm.x(), nrm.y(), nrm.z());
                cp.impulse  = pt.getAppliedImpulse();
                cp.distance = pt.getDistance();
                contactsAtoB.push_back(cp);
            }

            if (contactsAtoB.empty()) continue;

            CollisionInfo infoForA;
            infoForA.gameObject       = goB;
            infoForA.relativeVelocity = relVel;
            infoForA.contacts         = contactsAtoB;

            CollisionInfo infoForB;
            infoForB.gameObject       = goA;
            infoForB.relativeVelocity = -relVel;
            for (const auto& cp : contactsAtoB) {
                ContactPoint flipped = cp;
                flipped.normal       = -flipped.normal;
                infoForB.contacts.push_back(flipped);
            }

            
            bool wasInContact = previousContacts.count(pair) > 0;

            if (wasInContact) {
                goA->OnCollisionStay(infoForA);
                goB->OnCollisionStay(infoForB);
            } else {
                goA->OnCollisionEnter(infoForA);
                goB->OnCollisionEnter(infoForB);
            }
        }

        for (const auto& prevPair : previousContacts) {
            if (currentContacts.count(prevPair) == 0) {
                CollisionInfo exitInfoA;
                exitInfoA.gameObject = prevPair.second;

                CollisionInfo exitInfoB;
                exitInfoB.gameObject = prevPair.first;

                if (prevPair.first && prevPair.first->IsAlive())
                    prevPair.first->OnCollisionExit(exitInfoA);
                if (prevPair.second && prevPair.second->IsAlive())
                    prevPair.second->OnCollisionExit(exitInfoB);
            }
        }

        previousContacts = std::move(currentContacts);
    }

    void PhysicsManager::DispatchCollision(GameObject* target, const CollisionInfo& info, const char* callbackName) {
        // No longer used - callbacks are dispatched directly in ProcessCollisionCallbacks
        // Kept for API compatibility
    }

    void PhysicsManager::ClearCollisionState() {
        previousContacts.clear();
    }

    void PhysicsManager::AddRigidBody(RigidBody* pBody) {
        if (!dynamicsWorld || !pBody) {
            return;
        }

        if (pBody->GetRigidBody() && !pBody->IsAddedToWorld()) {
            btRigidBody* btBody = pBody->GetRigidBody();

            btBroadphaseProxy::CollisionFilterGroups collisionFilterGroup =
                btBody->getMass() == 0.0f ? btBroadphaseProxy::StaticFilter : btBroadphaseProxy::DefaultFilter;

            dynamicsWorld->addRigidBody(btBody, collisionFilterGroup, btBroadphaseProxy::AllFilter);


            pBody->SetAddedToWorld(true);
        }
    }

    void PhysicsManager::RemoveRigidBody(RigidBody* pBody) {
        if (!dynamicsWorld || !pBody) {
            return;
        }

        if (pBody->GetRigidBody() && pBody->IsAddedToWorld()) {
            dynamicsWorld->removeRigidBody(pBody->GetRigidBody());
            pBody->SetAddedToWorld(false);
        }
    }

    btDiscreteDynamicsWorld* PhysicsManager::GetPhyisicsWorld() const {
        return dynamicsWorld.get();
    }

    void PhysicsManager::SetDebugDrawEnabled(bool enabled) {
        debugDrawEnabled = enabled;
    }

    bool PhysicsManager::IsDebugDrawEnabled() const {
        return debugDrawEnabled;
    }

    void PhysicsManager::RenderDebug(const glm::mat4& viewProjection) {
        if (!debugDrawEnabled || !dynamicsWorld || !debugDrawer) {
            return;
        }

        debugDrawer->Begin();
        dynamicsWorld->debugDrawWorld();
        debugDrawer->End();
        debugDrawer->Render(viewProjection);
    }

    PhysicsDebugDrawer* PhysicsManager::GetDebugDrawer() const {
        return debugDrawer;
    }

} // namespace golias
