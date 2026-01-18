#pragma once

#include "core/math/aabb.h"

namespace golias {

    struct Plane {
        glm::vec3 normal;
        float d;

        float Distance(const glm::vec3& p) const;
    };

    struct Frustum {
        Plane planes[6];

        enum { FRUSTUM_LEFT = 0, FRUSTUM_RIGHT, FRUSTUM_BOTTOM, FRUSTUM_TOP, FRUSTUM_NEAR, FRUSTUM_FAR };

        bool IntersectsAABB(const AABB& aabb);

        bool IntersectsSphereAABB(const AABB& aabb);
    };

    Frustum CalculateFrustum(const glm::mat4& viewProj);

} // namespace golias
