#include "core/component/rigidbody_node.h"

#include "core/ember_core.h"


void RigidBody2D::input(const InputManager* input) {
    Node2D::input(input);
}

void RigidBody2D::draw_inspector() {
    Node2D::draw_inspector();
#if !defined(WITH_EDITOR)
    return;
#else

    bool needs_update = false;

    ImGui::Checkbox("Disabled", &is_disabled);

    if (ImGui::CollapsingHeader("Shape")) {

        const char* body_types[] = {"Static", "Dynamic", "Kinematic"};
        int type_idx             = static_cast<int>(collision_shape->body_type);
        if (ImGui::Combo("Body", &type_idx, body_types, IM_ARRAYSIZE(body_types))) {
            collision_shape->body_type = static_cast<BodyType>(type_idx);
            needs_update               = true;
        }

        const char* shape_types[] = {"Rectangle", "Circle", "Polygon", "Capsule"};
        int shape_idx             = static_cast<int>(collision_shape->shape_type);
        if (ImGui::Combo("Type", &shape_idx, shape_types, IM_ARRAYSIZE(shape_types))) {
            collision_shape->shape_type = static_cast<ShapeType>(shape_idx);
            needs_update                = true;
        }

        if (collision_shape->shape_type == ShapeType::RECTANGLE) {
            if (auto rect = dynamic_cast<RectangleShape*>(this->collision_shape.get())) {
                if (ImGui::DragFloat2("Size", &rect->size.x, 1.0f, 0.0f, FLT_MAX, "%.2f")) {
                    needs_update = true;
                }
            }


        } else if (collision_shape->shape_type == ShapeType::CIRCLE) {

            if (auto circ = dynamic_cast<CircleShape*>(this->collision_shape.get())) {
                if (ImGui::DragFloat("Radius", &circ->radius, 0.5f, 0.0f, FLT_MAX, "%.2f")) {
                    needs_update = true;
                }
            }
        }

        if (ImGui::CollapsingHeader("Material")) {
            if (ImGui::DragFloat("Density", &density, 0.1f, 0.0f, FLT_MAX) || ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 1.0f)
                || ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) {
                needs_update = true;
            }
        }

        if (ImGui::Checkbox("Sensor (?)", &is_sensor)) {
            needs_update = true;
        }
        if (ImGui::Checkbox("Fixed Rotation", &is_fixed_rotation)) {
            needs_update = true;
        }

        if (ImGui::DragFloat2("Offset", &collision_shape->offset.x, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
        }
    }

    float col[4] = {debug_color.r, debug_color.g, debug_color.b, debug_color.a};
    if (ImGui::ColorEdit4("Debug Color", col)) {
        debug_color = glm::vec4(col[0], col[1], col[2], col[3]);
    }

    if (ImGui::CollapsingHeader("Collision")) {
        int l = static_cast<int>(layer);
        if (ImGui::InputInt("Layer", &l)) {
            if (l >= 0 && l < 16) {
                layer        = static_cast<uint8_t>(l);
                needs_update = true;
            }
        }

        ImGui::Text("Mask");
        for (int i = 0; i < 16; i++) {
            bool bit = (collision_mask & (1 << i)) != 0;
            if (ImGui::Checkbox(("" + std::to_string(i)).c_str(), &bit)) {
                if (bit) {
                    collision_mask |= (1 << i);
                } else {
                    collision_mask &= ~(1 << i);
                }
                needs_update = true;
            }

            if (i % 4 != 3) {
                ImGui::SameLine();
            }
        }
    }

    if (needs_update) {
        update_body();
    }

#endif
}


void RigidBody2D::draw_hierarchy() {
    Node2D::draw_hierarchy();
}


void RigidBody2D::ready() {

    if (_is_ready) {
        return;
    }

    _is_ready = true;

    // LOG_INFO("Rigidbody::ready()");


    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type      = (collision_shape->body_type == BodyType::STATIC)  ? b2_staticBody
                       : (collision_shape->body_type == BodyType::DYNAMIC) ? b2_dynamicBody
                                                                           : b2_kinematicBody;

    body_def.position = pixels_to_world(get_global_transform().position + collision_shape->offset);

    body_id = b2CreateBody(GEngine->get_physics_world(), &body_def);

    if (B2_IS_NULL(body_id)) {
        LOG_ERROR("Failed to create RigidBody2D");
        return;
    }


    b2ShapeDef shapeDef = b2DefaultShapeDef();

    shapeDef.filter.categoryBits = 1 << layer;
    shapeDef.filter.maskBits     = collision_mask;

    shapeDef.density              = density;
    shapeDef.material.friction    = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor             = is_sensor;

    if (collision_shape->body_type == BodyType::DYNAMIC) {
        b2Body_SetFixedRotation(body_id, is_fixed_rotation);
    }

    if (collision_shape->shape_type == ShapeType::RECTANGLE) {
        if (auto* rect = dynamic_cast<RectangleShape*>(collision_shape.get())) {
            b2Polygon shape = b2MakeBox(rect->size.x * METERS_PER_PIXEL * 0.5f, rect->size.y * METERS_PER_PIXEL * 0.5f);

            rect->id = b2CreatePolygonShape(body_id, &shapeDef, &shape);
        }
    }

    if (collision_shape->shape_type == ShapeType::CIRCLE) {
        if (auto* circ = dynamic_cast<CircleShape*>(collision_shape.get())) {
            b2Circle shape;
            shape.center = {0, 0};
            shape.radius = circ->radius * METERS_PER_PIXEL;

            circ->id = b2CreateCircleShape(body_id, &shapeDef, &shape);
        }
    }

    b2Body_SetUserData(body_id, this);

    Node2D::ready();

}

void RigidBody2D::process(double delta_time) {
    Node2D::process(delta_time);

    if (B2_IS_NULL(body_id)) {
        return;
    }

    b2Vec2 world_pos    = b2Body_GetPosition(body_id);
    glm::vec2 pixel_pos = world_to_pixels(world_pos) - collision_shape->offset;
    b2Rot rot           = b2Body_GetRotation(body_id);
    float angle         = b2Rot_GetAngle(rot);
    set_transform({pixel_pos, get_transform().scale, -angle});
}

void RigidBody2D::draw(Renderer* renderer) {
    Node2D::draw(renderer);


    if (GEngine->Config.get_application().is_debug) {

        if (!B2_IS_NON_NULL(body_id)) {
            return;
        }

        glm::vec2 pos = world_to_pixels(b2Body_GetPosition(body_id));

        if (collision_shape->shape_type == ShapeType::RECTANGLE) {
            if (const auto rect = dynamic_cast<RectangleShape*>(this->collision_shape.get()); rect) {
                renderer->draw_rect({pos.x - rect->size.x / 2, pos.y - rect->size.y / 2, rect->size.x, rect->size.y},
                                    get_transform().rotation, debug_color, true, 1000);
            }


        } else if (collision_shape->shape_type == ShapeType::CIRCLE) {
            if (const auto circ = dynamic_cast<CircleShape*>(this->collision_shape.get()); circ) {

                renderer->draw_circle(pos.x, pos.y, get_transform().rotation, circ->radius, debug_color, true, 32, 1000);
            }
        }
    }
}

RigidBody2D::~RigidBody2D() {
}

void RigidBody2D::update_body() {
    if (!B2_IS_NON_NULL(body_id)) {
        return;
    }


    b2Body_SetType(body_id, (collision_shape->body_type == BodyType::STATIC)    ? b2_staticBody
                            : (collision_shape->body_type == BodyType::DYNAMIC) ? b2_dynamicBody
                                                                                : b2_kinematicBody);

    b2ShapeDef shapeDef = b2DefaultShapeDef();

    shapeDef.filter.categoryBits = 1 << layer;
    shapeDef.filter.maskBits     = collision_mask;

    shapeDef.density              = density;
    shapeDef.material.friction    = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor             = is_sensor;

    if (collision_shape->body_type == BodyType::DYNAMIC) {
        b2Body_SetFixedRotation(body_id, is_fixed_rotation);
    }

    if (collision_shape->shape_type == ShapeType::RECTANGLE) {
        if (auto rect = dynamic_cast<RectangleShape*>(this->collision_shape.get())) {

            b2Polygon shape = b2MakeBox(rect->size.x * METERS_PER_PIXEL * 0.5f, rect->size.y * METERS_PER_PIXEL * 0.5f);

            b2DestroyShape(collision_shape->id, false);
            collision_shape->id = b2CreatePolygonShape(body_id, &shapeDef, &shape);
        }
    }


    if (collision_shape->shape_type == ShapeType::CIRCLE) {
        b2Circle shape;
        shape.center          = {0, 0};
        if (auto circ = dynamic_cast<CircleShape*>(this->collision_shape.get())) {
            shape.radius          = circ->radius * METERS_PER_PIXEL;
            b2DestroyShape(collision_shape->id, false);
            collision_shape->id = b2CreateCircleShape(body_id, &shapeDef, &shape);
            // collision_shape.radius = circ->radius;
        }

    }
}

void RigidBody2D::apply_impulse(const glm::vec2& impulse) const {
    if (B2_IS_NON_NULL(body_id)) {
        b2Vec2 world_impulse = {impulse.x * METERS_PER_PIXEL, impulse.y * METERS_PER_PIXEL};
        b2Body_ApplyLinearImpulseToCenter(body_id, world_impulse, true);
    }
}

void RigidBody2D::apply_force(const glm::vec2& force) const {
    if (B2_IS_NON_NULL(body_id)) {
        b2Vec2 world_force = {force.x * METERS_PER_PIXEL, force.y * METERS_PER_PIXEL};
        b2Body_ApplyForceToCenter(body_id, world_force, true);
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
