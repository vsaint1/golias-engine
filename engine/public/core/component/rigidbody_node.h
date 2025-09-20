#pragma once
#include "phys_obj.h"




/**
 * @brief 2D rigid body with physics properties.
 *
 * @details
 * - Supports 16 layers (0-16) and collision masks.
 * - Can be static, dynamic, or kinematic.
 * - Supports rectangle, circle and polygon shapes.
 *
 * @version  1.2.0
 */
class RigidBody2D final : public PhysicsObject2D {
public:

    float friction    = 0.3f;
    float density     = 1.0f;
    float restitution = 0.0f;

    bool is_sensor         = false;
    bool is_fixed_rotation = false;

    void ready() override;
    void process(double delta_time) override;
    void draw(Renderer* renderer) override;

    void apply_impulse(const glm::vec2& impulse) const;
    void apply_force(const glm::vec2& force) const;

    void set_velocity(const glm::vec2& velocity) const;
    [[nodiscard]] glm::vec2 get_velocity() const;
    [[nodiscard]] bool is_on_ground() const;

    void draw_inspector() override;
    void draw_hierarchy() override;

    ~RigidBody2D() override;


private:
    void update_body();
    uint8_t layer           = 0; // < (0-15)
    uint16_t collision_mask = 0xFFFF; // 16 bits for layers
};
