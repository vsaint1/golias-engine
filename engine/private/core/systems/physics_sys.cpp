#include "core/systems/physics_sys.h"

#include "core/component/rigidbody_node.h"
#include "core/engine.h"

bool PhysicsManager::initialize() {


    return true;
}

void PhysicsManager::register_body(PhysicsObject2D* body) {
    if (body) {
        this->rigid_bodies.insert(body);
    }
}

void PhysicsManager::unregister_body(PhysicsObject2D* body) {
    if (body) {
        rigid_bodies.erase(body);
    }
}


void PhysicsManager::update(double delta_time) {
    for (PhysicsObject2D* body : rigid_bodies) {
        if (!body || !B2_IS_NON_NULL(body->body_id)) {
            continue;
        }


        b2ContactData contacts[16];
        int numContacts = b2Body_GetContactData(body->body_id, contacts, 16);

        std::unordered_set<PhysicsObject2D*> collidingWith;

        for (int i = 0; i < numContacts; ++i) {
            b2BodyId bodyA = b2Shape_GetBody(contacts[i].shapeIdA);
            b2BodyId bodyB = b2Shape_GetBody(contacts[i].shapeIdB);

            PhysicsObject2D* rbA = static_cast<PhysicsObject2D*>(b2Body_GetUserData(bodyA));
            PhysicsObject2D* rbB = static_cast<PhysicsObject2D*>(b2Body_GetUserData(bodyB));

            if (!rbA || !rbB) {
                continue;
            }

            PhysicsObject2D* other = (rbA == body) ? rbB : rbA;

            collidingWith.insert(other);
        }

        for (PhysicsObject2D* other : collidingWith) {
            if (!body->overlapping.contains(other)) {
                body->overlapping.insert(other);
                other->overlapping.insert(body);

                for (auto& cb : body->on_enter_callbacks) {
                    cb(other);
                }
                for (auto& cb : other->on_enter_callbacks) {
                    cb(body);
                }
            }
        }

        std::vector<PhysicsObject2D*> exited;
        for (PhysicsObject2D* other : body->overlapping) {
            if (!collidingWith.contains(other)) {
                exited.push_back(other);
            }
        }

        for (PhysicsObject2D* other : exited) {
            body->overlapping.erase(other);
            other->overlapping.erase(body);

            for (auto& cb : body->on_exit_callbacks) {
                cb(other);
            }
            for (auto& cb : other->on_exit_callbacks) {
                cb(body);
            }
        }
    }
}


void PhysicsManager::shutdown() {
    rigid_bodies.clear();
}
