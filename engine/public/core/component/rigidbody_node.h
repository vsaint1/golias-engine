#pragma once
#include "node.h"


enum class BodyType { STATIC, DYNAMIC, KINEMATIC };

/**
 * @brief 2D rigid body with physics properties.
 *
 * Supports 12 layers (0-11) and collision masks.
 */
class RigidBody2D final : public Node2D {
public:
    b2BodyId body_id = b2_nullBodyId;
    BodyType body_type = BodyType::STATIC;
    ShapeType shape_type = ShapeType::RECTANGLE;
    glm::vec2 body_size = glm::vec2(32.0f, 32.0f);
    glm::vec2 offset = glm::vec2(0.0f);

    float friction = 0.3f;
    float density = 1.0f;
    float restitution = 0.0f;
    float radius = 16.0f;
    bool is_sensor = false;
    bool is_disabled = false;
    bool is_fixed_rotation = false;

    glm::vec4 color = glm::vec4(1, 0, 0, 0.5f);


    void set_layer(uint8_t new_layer) {
        if (new_layer < 16) {
            layer = new_layer;
        }
    }

    void add_collision_layer(uint8_t target_layer);

    void remove_collision_layer(uint8_t target_layer);

    void set_collision_layers(const std::initializer_list<uint8_t>& layers);

    [[nodiscard]] uint8_t get_collision_layer() const;

    [[nodiscard]] uint16_t get_collision_mask() const;


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


private:
    uint8_t layer = 0;          // < (0-15)
    uint16_t collision_mask = 0xFFFF; // 16 bits for layers
};
