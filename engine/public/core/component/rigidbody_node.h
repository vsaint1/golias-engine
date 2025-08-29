#pragma once
#include "node.h"


enum class BodyType { STATIC, DYNAMIC, KINEMATIC };


class RigidBody2D final : public Node2D {
public:
    b2BodyId body_id       = b2_nullBodyId;
    BodyType body_type     = BodyType::STATIC;
    ShapeType shape_type   = ShapeType::RECTANGLE;
    glm::vec2 body_size    = glm::vec2(32.0f, 32.0f);
    glm::vec2 offset       = glm::vec2(0.0f);
    float friction         = 0.3f; // 0-1
    float density         = 1.0f; // mass per area
    float restitution      = 0.0f;
    float radius           = 16.0f;
    bool is_sensor      = false;
    bool is_disabled       = false;
    bool is_fixed_rotation = false;
    glm::vec4 color        = glm::vec4(1, 0, 0, 0.5f);


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

private:
    std::vector<std::function<void(Node2D*)>> on_enter_callbacks;
    std::vector<std::function<void(Node2D*)>> on_exit_callbacks;
    std::unordered_set<RigidBody2D*> currently_colliding;
};
