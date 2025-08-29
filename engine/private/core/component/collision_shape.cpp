#include "core/component/collision_shape.h"

#include "core/ember_core.h"
#include "core/systems/physics_sys.h"

CollisionShape2D::CollisionShape2D() {
}

CollisionShape2D::~CollisionShape2D() {
}

Rect2 CollisionShape2D::get_aabb() const {
    const glm::vec2 pos = get_global_transform().position;
    return Rect2(pos.x, pos.y, size.x, size.y);
}

glm::vec2 CollisionShape2D::get_center() const {
    const glm::vec2 pos = get_global_transform().position;
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

// TODO: check if debug mode is enabled
void CollisionShape2D::draw(Renderer* renderer) {


    switch (type) {
    case ShapeType::RECTANGLE:
        {
            Rect2 aabb = this->get_aabb();
            renderer->draw_rect({aabb.x, aabb.y, aabb.width, aabb.height}, 0, color, true, 100);
            break;
        }
    case ShapeType::CIRCLE:
        {
            const glm::vec2 center = this->get_center();
            renderer->draw_circle(center.x, center.y, 0, radius, color, true, 32, 100);
            break;
        }
    case ShapeType::POLYGON:
    case ShapeType::CAPSULE:
        break;
    }

    Node2D::draw(renderer);

}

void CollisionShape2D::input(const InputManager* input) {
    Node2D::input(input);
}
