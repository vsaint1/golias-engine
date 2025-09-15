#include "core/component/phys_obj.h"

#include "core/engine.h"


PhysicsObject2D::PhysicsObject2D() {
    GEngine->get_system<PhysicsManager>()->register_body(this);
}

PhysicsObject2D::~PhysicsObject2D() {

    GEngine->get_system<PhysicsManager>()->unregister_body(this);

    if (b2Body_IsValid(body_id)) {
        b2DestroyBody(body_id);
    }
}

void PhysicsObject2D::set_layer(uint8_t new_layer) {
    if (new_layer < 16) {
        layer = new_layer;
    }
}

void PhysicsObject2D::set_collision_mask(uint8_t bit_mask) {
    collision_mask = bit_mask;
}
void PhysicsObject2D::add_collision_layer(uint8_t target_layer) {
    if (target_layer < 16) {
        collision_mask |= (1 << target_layer);
    }
}

void PhysicsObject2D::remove_collision_layer(uint8_t target_layer) {
    if (target_layer < 16) {
        collision_mask &= ~(1 << target_layer);
    }
}

void PhysicsObject2D::set_collision_layers(const std::initializer_list<uint8_t>& layers) {
    collision_mask = 0;
    for (uint8_t l : layers) {
        if (l < 16) {
            collision_mask |= (1 << l);
        }
    }
}

uint8_t PhysicsObject2D::get_collision_layer() const {
    return layer;
}

uint16_t PhysicsObject2D::get_collision_mask() const {

    return collision_mask;
}

void PhysicsObject2D::on_body_entered(const std::function<void(Node2D*)>& callback) {
    on_enter_callbacks.push_back(callback);
}

void PhysicsObject2D::on_body_exited(const std::function<void(Node2D*)>& callback) {
    on_exit_callbacks.push_back(callback);
}
