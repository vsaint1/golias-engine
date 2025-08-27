#pragma once
#include "core/component/collision_shape.h"
#include "engine_sys.h"

class CollisionSystem final : public EngineSystem {
public:
    static std::vector<CollisionShape2D*> objects;

    static void register_object(CollisionShape2D* obj);
    static void unregister_object(CollisionShape2D* obj);

    bool check_aabb(const Rect2& a, const Rect2& b);
    bool check_circle(const glm::vec2& centerA, float radiusA,
                             const glm::vec2& centerB, float radiusB);
    bool check_circle_aabb(const glm::vec2& center, float radius, const Rect2& rect);
    bool check_collision(CollisionShape2D* a, CollisionShape2D* b);

    bool initialize() override;

    void update(double delta_time = 0) override;

    void shutdown() override;

    CollisionSystem()  = default;
    ~CollisionSystem() override = default;

};
