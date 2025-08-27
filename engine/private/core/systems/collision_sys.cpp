#include "core/systems/collision_sys.h"



std::vector<CollisionShape2D*> CollisionSystem::objects;


void CollisionSystem::register_object(CollisionShape2D* obj) {
    objects.push_back(obj);
}

void CollisionSystem::unregister_object(CollisionShape2D* obj) {
    std::erase(objects, obj);
}

bool CollisionSystem::check_aabb(const Rect2& a, const Rect2& b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

bool CollisionSystem::check_circle(const glm::vec2& centerA, float radiusA,
                                   const glm::vec2& centerB, float radiusB) {
    float dx = centerA.x - centerB.x;
    float dy = centerA.y - centerB.y;
    float dist_sq = dx * dx + dy * dy;
    float radius_sum = radiusA + radiusB;
    return dist_sq <= radius_sum * radius_sum;
}

bool CollisionSystem::check_circle_aabb(const glm::vec2& center, float radius, const Rect2& rect) {
    float closestX = glm::clamp(center.x, rect.x, rect.x + rect.width);
    float closestY = glm::clamp(center.y, rect.y, rect.y + rect.height);
    float dx = center.x - closestX;
    float dy = center.y - closestY;
    return dx*dx + dy*dy <= radius*radius;
}

bool CollisionSystem::check_collision(CollisionShape2D* a, CollisionShape2D* b) {
    if (!a || !b) return false;

    if (a->type == ShapeType::RECTANGLE && b->type == ShapeType::RECTANGLE) {
        return check_aabb(a->get_aabb(), b->get_aabb());
    }

    if (a->type == ShapeType::CIRCLE && b->type == ShapeType::CIRCLE) {
        return check_circle(a->get_center(), a->radius,
                            b->get_center(), b->radius);
    }

    if (a->type == ShapeType::CIRCLE && b->type == ShapeType::RECTANGLE) {
        return check_circle_aabb(a->get_center(), a->radius, b->get_aabb());
    }

    if (a->type == ShapeType::RECTANGLE && b->type == ShapeType::CIRCLE) {
        return check_circle_aabb(b->get_center(), b->radius, a->get_aabb());
    }

    return false;
}

void CollisionSystem::update() {
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            CollisionShape2D* a = objects[i];
            CollisionShape2D* b = objects[j];

            if (a->is_disabled || b->is_disabled) {
                continue;
            }

            bool colliding_now = check_collision(a, b);
            bool was_colliding = a->currently_colliding.contains(b);

            if (colliding_now && !was_colliding) {
                LOG_INFO("Collision started between objects %s and %s", a->get_name().c_str(), b->get_name().c_str());

                a->currently_colliding.insert(b);
                b->currently_colliding.insert(a);

                for (const auto& cb : a->on_enter_callbacks) {
                    cb(b);
                }
                for (const auto& cb : b->on_enter_callbacks) {
                    cb(a);
                }

            } else if (!colliding_now && was_colliding) {
                a->currently_colliding.erase(b);
                b->currently_colliding.erase(a);

                for (const auto& cb : a->on_exit_callbacks) {
                    cb(b);
                }
                for (const auto& cb : b->on_exit_callbacks) {
                    cb(a);
                }
            }
        }
    }
}
