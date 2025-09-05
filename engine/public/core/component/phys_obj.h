#pragma once

#include "collision_shapes.h"


/**
 * @brief 2D physical object node.
 *
 * @details
 * - Represents a physical object in 2D space
 *
 * @version 1.2.0
 */
class PhysicsObject2D : public Node2D {
public:
    PhysicsObject2D();


    void ready() override                    = 0;
    void process(double delta_time) override = 0;
    void draw(Renderer* renderer) override   = 0;

    void set_layer(uint8_t new_layer);
    void set_collision_mask(uint8_t mask);
    void add_collision_layer(uint8_t target_layer);
    void remove_collision_layer(uint8_t target_layer);

    void set_collision_layers(const std::initializer_list<uint8_t>& layers);

    [[nodiscard]] uint8_t get_collision_layer() const;

    [[nodiscard]] uint16_t get_collision_mask() const;
    ~PhysicsObject2D() override;

    void on_body_entered(const std::function<void(Node2D*)>& callback);

    void on_body_exited(const std::function<void(Node2D*)>& callback);

    std::unordered_set<PhysicsObject2D*> overlapping;
    std::vector<std::function<void(Node2D*)>> on_enter_callbacks;
    std::vector<std::function<void(Node2D*)>> on_exit_callbacks;

    b2BodyId body_id = b2_nullBodyId;

    std::unique_ptr<CollisionShape> collision_shape;

protected:
    bool is_disabled = false;

    glm::vec4 color = glm::vec4(1, 0, 0, 0.5f);

private:
    uint8_t layer           = 0; // < (0-15)
    uint16_t collision_mask = 0xFFFF; // 16 bits for layers
};
