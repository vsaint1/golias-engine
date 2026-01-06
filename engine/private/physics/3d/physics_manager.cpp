#include "physics/3d/physics_manager.h"

#include "core/engine.h"
#include "physics/3d/rigid_body.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


namespace golias {

    PhysicsManager::PhysicsManager()  = default;
    
    PhysicsManager::~PhysicsManager() {
        
        if (dynamicsWorld) {
            int numCollisionObjects = dynamicsWorld->getNumCollisionObjects();
            for (int i = numCollisionObjects - 1; i >= 0; --i) {
                btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
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
        constexpr btScalar FIXED_TIME_STEP = 1 / 60.0f;
        constexpr int MAX_SUB_STEPS        = 4;

        if (dynamicsWorld) {
            dynamicsWorld->stepSimulation(deltaTime, MAX_SUB_STEPS, FIXED_TIME_STEP);
        }
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
