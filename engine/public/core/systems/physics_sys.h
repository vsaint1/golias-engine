#pragma once
#include "core/component/rigidbody_node.h"
#include "engine_sys.h"


/*!
 * @brief PhysicsManager class
 *
 * @details Manages 2D physics Collision and Resolution using Box2D.
 *
 * @version 1.2.0
 */
class PhysicsManager final : public EngineManager {
public:
    std::unordered_set<PhysicsObject2D*> rigid_bodies;

    void register_body(PhysicsObject2D* body);

    void unregister_body(PhysicsObject2D* body);

    bool initialize() override;

    void update(double delta_time = 0) override;

    void shutdown() override;

    PhysicsManager()  = default;
    ~PhysicsManager() override = default;
protected:
    const char* name = "PhysicsSystem";

};
