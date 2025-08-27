#pragma once

#include "node.h"

enum class ShapeType {
    RECTANGLE,
    CIRCLE,
    POLYGON,
    CAPSULE

};


class CollisionShape2D final : public Node2D {
public:
    ShapeType type = ShapeType::RECTANGLE;
    glm::vec2 size{32, 32};
    float radius = 16.0f;
    glm::vec2 offset{0, 0};
    bool is_disabled = false;
    bool one_way_collision = false;
    glm::vec4 color = glm::vec4(1, 0,0, 0.5f);


    std::vector<std::function<void(CollisionShape2D*)>> on_enter_callbacks;
    std::vector<std::function<void(CollisionShape2D*)>> on_exit_callbacks;
    std::unordered_set<Node2D*> currently_colliding;


    CollisionShape2D();

    ~CollisionShape2D() override;


    Rect2 get_aabb() const;

    glm::vec2 get_center() const;

    void on_body_entered(const std::function<void(CollisionShape2D*)>& callback);

    void on_body_exited(const std::function<void(CollisionShape2D*)>& callback) ;

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void input(const InputManager* input) override;


};
