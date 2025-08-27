#include "core/component/collision_shape.h"

#include "core/systems/collision_sys.h"

CollisionShape2D::CollisionShape2D() {
    CollisionSystem::register_object(this);
}

CollisionShape2D::~CollisionShape2D() {
    CollisionSystem::unregister_object(this);
}

Rect2 CollisionShape2D::get_aabb() const {
    const glm::vec2 pos = get_global_transform().position + offset;
    return Rect2(pos.x, pos.y, size.x, size.y);
}

glm::vec2 CollisionShape2D::get_center() const {
    const glm::vec2 pos = get_global_transform().position + offset;
    return pos + size * 0.5f;
}

void CollisionShape2D::on_body_entered(const std::function<void(CollisionShape2D*)>& callback) {

    on_enter_callbacks.push_back(callback);
}

void CollisionShape2D::on_body_exited(const std::function<void(CollisionShape2D*)>& callback) {


    on_exit_callbacks.push_back(callback);
}


void CollisionShape2D::ready() {
    Node2D::ready();
}

void CollisionShape2D::process(double delta_time) {
    Node2D::process(delta_time);
}

void CollisionShape2D::draw(Renderer* renderer) {
    Node2D::draw(renderer);
}

void CollisionShape2D::input(const InputManager* input) {
    Node2D::input(input);
}
