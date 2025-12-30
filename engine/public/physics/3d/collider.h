#pragma once

#include <glm/glm.hpp>

class btCollisionShape;

namespace golias {

    class Collider {
    public:
        Collider(btCollisionShape* pShape);
        ~Collider();

        btCollisionShape* GetCollisionShape() const;

    protected:
        Collider() = default;
        btCollisionShape* shape;
    };


    class BoxCollider : public Collider {
    public:
        BoxCollider(const glm::vec3& extents);
        ~BoxCollider() = default;
    };

    class SphereCollider : public Collider {
    public:
        SphereCollider(float radius);
        ~SphereCollider() = default;
    };

    class CapsuleCollider : public Collider {
    public:
        CapsuleCollider(float radius, float height);
        ~CapsuleCollider() = default;
    };


} // namespace golias
