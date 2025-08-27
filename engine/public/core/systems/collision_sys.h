#pragma once
#include "core/component/collision_shape.h"

class CollisionSystem {
public:
    static std::vector<CollisionShape2D*> objects;

    static void register_object(CollisionShape2D* obj);
    static void unregister_object(CollisionShape2D* obj);

    static bool check_aabb(const Rect2& a, const Rect2& b);
    static bool check_circle(const glm::vec2& centerA, float radiusA,
                             const glm::vec2& centerB, float radiusB);
    static bool check_circle_aabb(const glm::vec2& center, float radius, const Rect2& rect);
    static bool check_collision(CollisionShape2D* a, CollisionShape2D* b);

    static void update();

};
