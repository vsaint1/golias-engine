#pragma once

#include <memory>

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;

namespace golias {

    class RigidBody;

    class PhysicsManager {
    public:
        PhysicsManager();
        ~PhysicsManager();

        bool Initialize();
        void StepSimulation(float deltaTime);

        btDiscreteDynamicsWorld* GetPhyisicsWorld() const;

        void AddRigidBody(RigidBody* pBody);
        void RemoveRigidBody(RigidBody* pBody);

    private:
        std::unique_ptr<btBroadphaseInterface> broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
        // std::unique_ptr<btIDebugDraw> debugDrawer;
    };
} // namespace golias
