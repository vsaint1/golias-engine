#pragma once
#include "core/component/rigidbody_node.h"
#include "engine_sys.h"

class PhysicsManager final : public EngineManager {
public:
    std::unordered_set<RigidBody2D*> rigid_bodies;

    void register_body(RigidBody2D* body);

    void unregister_body(RigidBody2D* body);

    bool initialize() override;

    void update(double delta_time = 0) override;

    void shutdown() override;

    PhysicsManager()  = default;
    ~PhysicsManager() override = default;
protected:
    const char* name = "PhysicsSystem";

};
