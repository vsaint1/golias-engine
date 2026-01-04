#pragma once

#include <memory>
#include <glm/glm.hpp>

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btIDebugDraw;

namespace golias {

    class RigidBody;
    class PhysicsDebugDrawer;

    class PhysicsManager {
    public:
        PhysicsManager();
        ~PhysicsManager();

        bool Initialize(PhysicsDebugDrawer* pDebugDrawer = nullptr);
        void StepSimulation(float deltaTime);

        btDiscreteDynamicsWorld* GetPhyisicsWorld() const;

        void AddRigidBody(RigidBody* pBody);
        void RemoveRigidBody(RigidBody* pBody);

        void SetDebugDrawEnabled(bool enabled);
        bool IsDebugDrawEnabled() const;
        void RenderDebug(const glm::mat4& viewProjection);
        PhysicsDebugDrawer* GetDebugDrawer() const;

    private:
        std::unique_ptr<btBroadphaseInterface> broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
        PhysicsDebugDrawer* debugDrawer = nullptr;
        bool debugDrawEnabled = false;
    };
} // namespace golias
