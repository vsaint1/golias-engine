#pragma once
#include "node.h"


enum class BodyType { STATIC, DYNAMIC, KINEMATIC };


/**
 * @brief 2D rigid body with physics properties.
 *
 */
class RigidBody2D final : public Node2D {
public:
    /**
     * @brief Physics engine body identifier.
     * @note b2_nullBodyId if the body is not yet created.
     */
    b2BodyId body_id = b2_nullBodyId;

    /**
     * @brief Type of the Physics body
     */
    BodyType body_type = BodyType::STATIC;

    /**
     * @brief Shape of the rigid body for collision detection.
     */
    ShapeType shape_type = ShapeType::RECTANGLE;

    /**
     * @brief Size of the body in world units (width, height).
     */
    glm::vec2 body_size = glm::vec2(32.0f, 32.0f);

    /**
     * @brief Offset of the physics body relative to the node's origin.
     */
    glm::vec2 offset = glm::vec2(0.0f);

    /**
     * @brief Friction coefficient of the body (0 = no friction, 1 = high friction).
     */
    float friction = 0.3f;

    /**
     * @brief Density of the body (mass per unit area).
     */
    float density = 1.0f;

    /**
     * @brief Restitution coefficient (bounciness, 0 = no bounce, 1 = full bounce).
     */
    float restitution = 0.0f;

    /**
     * @brief Radius of the body.
     * @note Used when shape_type is CIRCLE.
     */
    float radius = 16.0f;

    /**
     * @brief If true, the body is a sensor and does not generate collision response.
     */
    bool is_sensor = false;

    /**
     * @brief If true, the body is temporarily disabled in the physics simulation.
     */
    bool is_disabled = false;

    /**
     * @brief If true, the body's rotation is fixed and cannot rotate.
     */
    bool is_fixed_rotation = false;

    /**
       * @brief Debug/visualization color
       */
    glm::vec4 color = glm::vec4(1, 0, 0, 0.5f);

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void apply_impulse(const glm::vec2& impulse) const;

    void set_velocity(const glm::vec2& velocity) const;

    glm::vec2 get_velocity() const;

    bool is_on_ground() const;

    void on_body_entered(const std::function<void(Node2D*)>& callback);

    void on_body_exited(const std::function<void(Node2D*)>& callback);

    ~RigidBody2D() override;

    std::unordered_set<RigidBody2D*> currently_colliding;
    std::vector<std::function<void(Node2D*)>> on_enter_callbacks;
    std::vector<std::function<void(Node2D*)>> on_exit_callbacks;

};
