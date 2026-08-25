#pragma once
#include "stdafx.h"

class btCollisionShape;

namespace golias {

    /// @brief  A base class for colliders that can be attached to physics objects.
    class Collider {
    public:
        Collider();
        ~Collider(); 

        btCollisionShape* GetShape() const;

    protected:
        btCollisionShape* mShape = nullptr;
    };


    /// @brief A collider that represents a box shape.
    class BoxCollider : public Collider {
    public:
        BoxCollider(float width, float height, float depth);
        BoxCollider(const glm::vec3& extents);


    private:
        float mWidth  = 0.0f;
        float mHeight = 0.0f;
        float mDepth  = 0.0f;
    };


    /// @brief A collider that represents a sphere shape.
    class SphereCollider : public Collider {
    public:
        SphereCollider(float radius);

    private:
        float mRadius = 0.0f;
    };


    class CapsuleCollider : public Collider {
    public:
        CapsuleCollider(float radius, float height);

    private:
        float mRadius = 0.0f;
        float mHeight = 0.0f;
    };
} // namespace golias
