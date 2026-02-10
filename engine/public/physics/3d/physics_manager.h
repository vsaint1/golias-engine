#pragma once

#include "physics/3d/collision_info.h"
#include <memory>
#include <set>
#include <unordered_map>
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
    class GameObject;

    /// @brief Ordered pair of GameObjects for collision tracking (A < B by pointer).
    using CollisionPair = std::pair<GameObject*, GameObject*>;

    class PhysicsManager {
    public:
        PhysicsManager();
        ~PhysicsManager();

        bool Initialize(PhysicsDebugDrawer* pDebugDrawer = nullptr);
        void StepSimulation(float deltaTime);

        btDiscreteDynamicsWorld* GetPhyisicsWorld() const;

        void AddRigidBody(RigidBody* pBody);
        void RemoveRigidBody(RigidBody* pBody);

        /// @brief Clear collision tracking state. Call when switching scenes to
        ///        prevent dangling GameObject* pointers in previousContacts.
        void ClearCollisionState();

        void SetDebugDrawEnabled(bool enabled);
        bool IsDebugDrawEnabled() const;
        void RenderDebug(const glm::mat4& viewProjection);
        PhysicsDebugDrawer* GetDebugDrawer() const;

    private:
        /// Process contact manifolds and dispatch Enter/Stay/Exit callbacks.
        void ProcessCollisionCallbacks();

        /// Dispatch collision info to all Behaviour components on a GameObject.
        void DispatchCollision(GameObject* target, const CollisionInfo& info, const char* callbackName);

        std::unique_ptr<btBroadphaseInterface> broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
        PhysicsDebugDrawer* debugDrawer = nullptr;
        bool debugDrawEnabled = false;

        /// Pairs that were in contact during the previous physics step.
        std::set<CollisionPair> previousContacts;
    };
} // namespace golias
