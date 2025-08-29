#include "core/component/rigidbody_node.h"

#include "core/ember_core.h"

void RigidBody2D::on_body_entered(const std::function<void(Node2D*)>& callback) {
    on_enter_callbacks.push_back(callback);
}

void RigidBody2D::on_body_exited(const std::function<void(Node2D*)>& callback) {
    on_exit_callbacks.push_back(callback);
}


void RigidBody2D::ready() {

    Node2D::ready();

    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = (body_type == BodyType::STATIC) ? b2_staticBody : (body_type == BodyType::DYNAMIC) ? b2_dynamicBody : b2_kinematicBody;

    body_def.position = pixels_to_world(get_global_transform().position + offset);

    body_id = b2CreateBody(GEngine->get_physics_world(), &body_def);

    b2ShapeDef shapeDef = b2DefaultShapeDef();


    shapeDef.density              = density;
    shapeDef.material.friction    = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor             = is_sensor;

    if (body_type == BodyType::DYNAMIC) {
        b2Body_SetFixedRotation(body_id,is_fixed_rotation);
    }


    if (shape_type == ShapeType::RECTANGLE) {
        b2Polygon shape = b2MakeBox(body_size.x * METERS_PER_PIXEL * 0.5f, body_size.y * METERS_PER_PIXEL * 0.5f);

        b2CreatePolygonShape(body_id, &shapeDef, &shape);
    }

    if (shape_type == ShapeType::CIRCLE) {
        b2Circle shape;
        shape.center = {0, 0};
        shape.radius = radius * METERS_PER_PIXEL;

        b2CreateCircleShape(body_id, &shapeDef, &shape);
    }

    b2Body_SetUserData(body_id, this);
}

void RigidBody2D::process(double delta_time) {
    Node2D::process(delta_time);

    if (!B2_IS_NON_NULL(body_id)) {
        return;
    }

    b2Vec2 world_pos    = b2Body_GetPosition(body_id);
    glm::vec2 pixel_pos = world_to_pixels(world_pos) - offset;
    b2Rot rot = b2Body_GetRotation(body_id);
    float angle = std::atan2(rot.s, rot.c);

    set_transform({pixel_pos, get_transform().scale, -angle});

    b2ContactData contacts[16];
    int numContacts = b2Body_GetContactData(body_id, contacts, 16);

    std::unordered_set<RigidBody2D*> nowColliding;

    for (int i = 0; i < numContacts; ++i) {
        b2BodyId bodyA = b2Shape_GetBody(contacts[i].shapeIdA);
        b2BodyId bodyB = b2Shape_GetBody(contacts[i].shapeIdB);

        RigidBody2D* rbA = static_cast<RigidBody2D*>(b2Body_GetUserData(bodyA));
        RigidBody2D* rbB = static_cast<RigidBody2D*>(b2Body_GetUserData(bodyB));

        RigidBody2D* otherRB = (rbA == this) ? rbB : rbA;
        if (!otherRB) {
            continue;
        }

        nowColliding.insert(otherRB);

        if (!currently_colliding.contains(otherRB)) {
            currently_colliding.insert(otherRB);
            otherRB->currently_colliding.insert(this);

            for (auto& cb : on_enter_callbacks) {
                cb(otherRB);
            }
            for (auto& cb : otherRB->on_enter_callbacks) {
                cb(this);
            }
        }
    }

    std::vector<RigidBody2D*> exited;
    for (RigidBody2D* otherRB : currently_colliding) {
        if (!nowColliding.contains(otherRB)) {
            exited.push_back(otherRB);
        }
    }

    for (RigidBody2D* otherRB : exited) {
        currently_colliding.erase(otherRB);
        otherRB->currently_colliding.erase(this);

        for (auto& cb : on_exit_callbacks) {
            cb(otherRB);
        }
        for (auto& cb : otherRB->on_exit_callbacks) {
            cb(this);
        }
    }
}

void RigidBody2D::draw(Renderer* renderer) {
    Node2D::draw(renderer);

    const glm::vec2 pos = world_to_pixels(b2Body_GetPosition(body_id));

    if (shape_type == ShapeType::RECTANGLE) {
        renderer->draw_rect({pos.x - body_size.x / 2, pos.y - body_size.y / 2, body_size.x, body_size.y}, get_transform().rotation, color, true, 1000);
    } else if (shape_type == ShapeType::CIRCLE) {
        renderer->draw_circle(pos.x, pos.y, get_transform().rotation, radius, color, true, 32, 1000);
    }
}

RigidBody2D::~RigidBody2D() {
}


void RigidBody2D::apply_impulse(const glm::vec2& impulse) const {
    if (B2_IS_NON_NULL(body_id)) {
        b2Vec2 world_impulse = {impulse.x * METERS_PER_PIXEL, impulse.y * METERS_PER_PIXEL};
        b2Body_ApplyLinearImpulseToCenter(body_id, world_impulse, true);
    }
}

void RigidBody2D::set_velocity(const glm::vec2& velocity) const {
    if (B2_IS_NON_NULL(body_id)) {
        b2Vec2 world_velocity = {velocity.x * METERS_PER_PIXEL, velocity.y * METERS_PER_PIXEL};
        b2Body_SetLinearVelocity(body_id, world_velocity);
    }
}

glm::vec2 RigidBody2D::get_velocity() const {
    if (B2_IS_NON_NULL(body_id)) {
        b2Vec2 world_velocity = b2Body_GetLinearVelocity(body_id);
        return glm::vec2(world_velocity.x * PIXELS_PER_METER, world_velocity.y * PIXELS_PER_METER);
    }
    return glm::vec2(0.0f);
}

bool RigidBody2D::is_on_ground() const {
    if (!B2_IS_NON_NULL(body_id)) {
        return false;
    }

    b2ContactData contacts[16];
    int contact_data = b2Body_GetContactData(body_id, contacts, 16);

    for (int i = 0; i < contact_data; ++i) {
        if (contacts[i].manifold.pointCount > 0) {
            b2Vec2 normal = contacts[i].manifold.normal;

            if (fabsf(normal.y) > 0.7f) {
                b2Vec2 pos = b2Body_GetPosition(body_id);
                b2Vec2 cp  = contacts[i].manifold.points[0].point;
                if ((pos.y - cp.y) > 0.0f) {
                    return true;
                }
            }
        }
    }
    return false;
}
