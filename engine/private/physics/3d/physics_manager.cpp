#include "physics/3d/physics_manager.h"

#include "physics/3d/rigid_body.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>


namespace golias {

    PhysicsManager::PhysicsManager()  = default;
    PhysicsManager::~PhysicsManager() = default;

    bool PhysicsManager::Initialize() {

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

        if (pBody->GetdBody() && !pBody->IsAddedToWorld()) {
            btRigidBody* btBody = pBody->GetdBody();


            btBroadphaseProxy::CollisionFilterGroups collisionFilterGroup =
                btBody->getMass() == 0.0f ? btBroadphaseProxy::DefaultFilter : btBroadphaseProxy::StaticFilter;

            dynamicsWorld->addRigidBody(btBody, collisionFilterGroup, btBroadphaseProxy::AllFilter);


            pBody->SetAddedToWorld(true);
        }
    }

    void PhysicsManager::RemoveRigidBody(RigidBody* pBody) {
        if (!dynamicsWorld || !pBody) {
            return;
        }

        if (pBody->GetdBody() && pBody->IsAddedToWorld()) {
            dynamicsWorld->removeRigidBody(pBody->GetdBody());
            pBody->SetAddedToWorld(false);
        }
    }

    btDiscreteDynamicsWorld* PhysicsManager::GetPhyisicsWorld() const {
        return dynamicsWorld.get();
    }

} // namespace golias
