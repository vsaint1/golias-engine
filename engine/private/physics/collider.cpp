#include "physics/collider.h"

#include <btBulletCollisionCommon.h>

namespace golias {

    Collider::Collider() {
    }

    Collider::~Collider() {

        if (mShape) {
            delete mShape;
            mShape = nullptr;
        }
    }

    btCollisionShape* Collider::GetShape() const {
        return mShape;
    }


    BoxCollider::BoxCollider(float width, float height, float depth) : mWidth(width), mHeight(height), mDepth(depth) {
        glm::vec3 halfExtents(width / 2.0f, height / 2.0f, depth / 2.0f);
        mShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    }

    BoxCollider::BoxCollider(const glm::vec3& extents) : mWidth(extents.x), mHeight(extents.y), mDepth(extents.z) {
        glm::vec3 halfExtents(extents.x / 2.0f, extents.y / 2.0f, extents.z / 2.0f);
        mShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    }


    SphereCollider::SphereCollider(float radius) : mRadius(radius) {
        mShape = new btSphereShape(radius);
    }


    CapsuleCollider::CapsuleCollider(float radius, float height) : mRadius(radius), mHeight(height) {
        mShape = new btCapsuleShape(radius, height);
    }


} // namespace golias
