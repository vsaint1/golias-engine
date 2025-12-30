#include "physics/3d/collider.h"

#include <btBulletCollisionCommon.h>

namespace golias {
    Collider::Collider(btCollisionShape* pShape) : shape(pShape) {
    }

    Collider::~Collider() {
        delete shape;
    }

    btCollisionShape* Collider::GetCollisionShape() const {
        return shape;
    }

    BoxCollider::BoxCollider(const glm::vec3& extents) {
        btVector3 halfExtents(extents.x * 0.5f, extents.y * 0.5f, extents.z * 0.5f);
        shape = new btBoxShape(halfExtents);
    }

    SphereCollider::SphereCollider(float radius) {
        shape = new btSphereShape(radius);
    }

    CapsuleCollider::CapsuleCollider(float radius, float height) {
        shape = new btCapsuleShape(radius, height);
    }
} // namespace golias
