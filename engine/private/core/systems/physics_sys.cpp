#include "core/component/rigidbody_node.h"
#include "core/engine.h"
#include "core/systems/physics_sys.h"

bool PhysicsManager::initialize() {


    return true;
}

void PhysicsManager::register_body(RigidBody2D* body) {
    if (body) {
        this->rigid_bodies.insert(body);
    }
}

void PhysicsManager::unregister_body(RigidBody2D* body) {
    if (body) rigid_bodies.erase(body);
}

void PhysicsManager::update(double delta_time) {

     for (RigidBody2D* body : rigid_bodies) {
            if (!body || !B2_IS_NON_NULL(body->body_id)) continue;

            b2Vec2 world_pos = b2Body_GetPosition(body->body_id);
            glm::vec2 pixel_pos = world_to_pixels(world_pos) - body->offset;
            b2Rot rot = b2Body_GetRotation(body->body_id);
            float angle = b2Rot_GetAngle(rot);
            body->set_transform({pixel_pos, body->get_transform().scale, -angle});

            b2ContactData contacts[16];
            int numContacts = b2Body_GetContactData(body->body_id, contacts, 16);

            std::unordered_set<RigidBody2D*> collidingWith;

            for (int i = 0; i < numContacts; ++i) {
                b2BodyId bodyA = b2Shape_GetBody(contacts[i].shapeIdA);
                b2BodyId bodyB = b2Shape_GetBody(contacts[i].shapeIdB);

                RigidBody2D* rbA = static_cast<RigidBody2D*>(b2Body_GetUserData(bodyA));
                RigidBody2D* rbB = static_cast<RigidBody2D*>(b2Body_GetUserData(bodyB));

                if (!rbA || !rbB) continue;

                RigidBody2D* other = (rbA == body) ? rbB : rbA;

                collidingWith.insert(other);
            }

            for (RigidBody2D* other : collidingWith) {
                if (!body->currently_colliding.contains(other)) {
                    body->currently_colliding.insert(other);
                    other->currently_colliding.insert(body);

                    for (auto& cb : body->on_enter_callbacks) cb(other);
                    for (auto& cb : other->on_enter_callbacks) cb(body);
                }
            }

            std::vector<RigidBody2D*> exited;
            for (RigidBody2D* other : body->currently_colliding) {
                if (!collidingWith.contains(other)) exited.push_back(other);
            }

            for (RigidBody2D* other : exited) {
                body->currently_colliding.erase(other);
                other->currently_colliding.erase(body);

                for (auto& cb : body->on_exit_callbacks) cb(other);
                for (auto& cb : other->on_exit_callbacks) cb(body);
            }
        }
}

void PhysicsManager::shutdown() {
    rigid_bodies.clear();
}
