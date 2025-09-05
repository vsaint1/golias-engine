#pragma once

#include "node.h"

enum class BodyType { STATIC, DYNAMIC, KINEMATIC };

enum class ShapeType {
    RECTANGLE,
    CIRCLE,
    POLYGON,
    CAPSULE

};

struct CollisionShape {
    b2ShapeId id         = b2_nullShapeId;
    BodyType body_type   = BodyType::STATIC;
    ShapeType shape_type = ShapeType::RECTANGLE;

    glm::vec2 offset = glm::vec2(0.0f);

    virtual ~CollisionShape() = default;
};

struct RectangleShape final : CollisionShape {
    glm::vec2 size = glm::vec2(32.0f, 32.0f);

    explicit RectangleShape(const glm::vec2& size = glm::vec2(32.0f, 32.0f),
                            BodyType body_type = BodyType::STATIC,const glm::vec2& offset = glm::vec2(0.0f)) {
        this->size       = size;
        this->offset     = offset;
        this->body_type  = body_type;
        this->shape_type = ShapeType::RECTANGLE;
    }
};

struct CircleShape final : CollisionShape {
    float radius = 16.0f;

    explicit CircleShape(float radius = 16.0f, BodyType body_type = BodyType::STATIC,const glm::vec2& offset = glm::vec2(0.0f)) {
        this->radius     = radius;
        this->offset     = offset;
        this->body_type  = body_type;
        this->shape_type = ShapeType::CIRCLE;
    }
};

struct CapsuleShape final : CollisionShape {
    float radius = 16.0f;
    float height = 32.0f;

    explicit CapsuleShape(float radius = 16.0f, float height = 32.0f,
                          BodyType body_type = BodyType::STATIC,const glm::vec2& offset = glm::vec2(0.0f)) {
        this->radius     = radius;
        this->height     = height;
        this->offset     = offset;
        this->body_type  = body_type;
        this->shape_type = ShapeType::CAPSULE;
    }
};

struct PolygonShape final : CollisionShape {
    std::vector<glm::vec2> vertices;

    explicit PolygonShape(const std::vector<glm::vec2>& verts = {},
                          BodyType body_type = BodyType::STATIC,const glm::vec2& offset = glm::vec2(0.0f)) {
        this->vertices   = verts;
        this->offset     = offset;
        this->body_type  = body_type;
        this->shape_type = ShapeType::POLYGON;
    }
};
