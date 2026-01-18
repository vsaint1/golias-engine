#include "core/graphics/culling.h"


namespace golias {

    float Plane::Distance(const glm::vec3& p) const {
        return glm::dot(normal, p) + d;
    }



    
    Frustum CalculateFrustum(const glm::mat4& viewProj) {
        Frustum f;

        f.planes[Frustum::FRUSTUM_LEFT].normal.x = viewProj[0][3] + viewProj[0][0];
        f.planes[Frustum::FRUSTUM_LEFT].normal.y = viewProj[1][3] + viewProj[1][0];
        f.planes[Frustum::FRUSTUM_LEFT].normal.z = viewProj[2][3] + viewProj[2][0];
        f.planes[Frustum::FRUSTUM_LEFT].d        = viewProj[3][3] + viewProj[3][0];

        f.planes[Frustum::FRUSTUM_RIGHT].normal.x = viewProj[0][3] - viewProj[0][0];
        f.planes[Frustum::FRUSTUM_RIGHT].normal.y = viewProj[1][3] - viewProj[1][0];
        f.planes[Frustum::FRUSTUM_RIGHT].normal.z = viewProj[2][3] - viewProj[2][0];
        f.planes[Frustum::FRUSTUM_RIGHT].d        = viewProj[3][3] - viewProj[3][0];

        f.planes[Frustum::FRUSTUM_BOTTOM].normal.x = viewProj[0][3] + viewProj[0][1];
        f.planes[Frustum::FRUSTUM_BOTTOM].normal.y = viewProj[1][3] + viewProj[1][1];
        f.planes[Frustum::FRUSTUM_BOTTOM].normal.z = viewProj[2][3] + viewProj[2][1];
        f.planes[Frustum::FRUSTUM_BOTTOM].d        = viewProj[3][3] + viewProj[3][1];

        f.planes[Frustum::FRUSTUM_TOP].normal.x = viewProj[0][3] - viewProj[0][1];
        f.planes[Frustum::FRUSTUM_TOP].normal.y = viewProj[1][3] - viewProj[1][1];
        f.planes[Frustum::FRUSTUM_TOP].normal.z = viewProj[2][3] - viewProj[2][1];
        f.planes[Frustum::FRUSTUM_TOP].d        = viewProj[3][3] - viewProj[3][1];

        f.planes[Frustum::FRUSTUM_NEAR].normal.x = viewProj[0][3] + viewProj[0][2];
        f.planes[Frustum::FRUSTUM_NEAR].normal.y = viewProj[1][3] + viewProj[1][2];
        f.planes[Frustum::FRUSTUM_NEAR].normal.z = viewProj[2][3] + viewProj[2][2];
        f.planes[Frustum::FRUSTUM_NEAR].d        = viewProj[3][3] + viewProj[3][2];

        f.planes[Frustum::FRUSTUM_FAR].normal.x = viewProj[0][3] - viewProj[0][2];
        f.planes[Frustum::FRUSTUM_FAR].normal.y = viewProj[1][3] - viewProj[1][2];
        f.planes[Frustum::FRUSTUM_FAR].normal.z = viewProj[2][3] - viewProj[2][2];
        f.planes[Frustum::FRUSTUM_FAR].d        = viewProj[3][3] - viewProj[3][2];

        // Normalize planes
        for (int i = 0; i < 6; ++i) {
            float len = glm::length(f.planes[i].normal);
  
            f.planes[i].normal /= len;
            f.planes[i].d /= len;
        }

        return f;
    }

    bool Frustum::IntersectsAABB(const AABB& aabb) {
        const glm::vec3 center  = aabb.Center();
        const glm::vec3 extents = aabb.Extents();

        for (int i = 0; i < 6; ++i) {
            const Plane& p = planes[i];

            float r = extents.x * std::abs(p.normal.x) + extents.y * std::abs(p.normal.y) + extents.z * std::abs(p.normal.z);

            float d = glm::dot(p.normal, center) + p.d;

            if (d + r < 0.0f) {
                return false;
            }
        }

        return true;
    }

    bool Frustum::IntersectsSphereAABB(const AABB& aabb) {
        const glm::vec3 center = aabb.Center();
        const float radius     = aabb.Radius();

        for (int i = 0; i < 6; ++i) {
            if (planes[i].Distance(center) < -radius) {
                return false;
            }
        }
        return true;
    }

} // namespace golias
