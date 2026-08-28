#include "math/aabb.h"

namespace golias {

    glm::vec3 AABB::GetMin() const {
        return mMin;
    }

    glm::vec3 AABB::GetMax() const {
        return mMax;
    }

    bool AABB::IsValid() const {
        return mMin.x <= mMax.x && mMin.y <= mMax.y && mMin.z <= mMax.z;
    }

    void AABB::Expand(const glm::vec3& point) {
        mMin = glm::min(mMin, point);
        mMax = glm::max(mMax, point);
    }

    glm::vec3 AABB::GetCenter() const {
        return (mMin + mMax) * 0.5f;
    }

    glm::vec3 AABB::GetExtents() const {
        return (mMax - mMin) * 0.5f;
    }

    AABB AABB::Transformed(const glm::mat4& transform) const {
        if (!IsValid()) {
            return {};
        }


        const glm::vec3 center  = GetCenter();
        const glm::vec3 extents = GetExtents();

        const glm::mat3 linear(transform);

        const glm::vec3 transformedCenter = glm::vec3(transform * glm::vec4(center, 1.0f));

        const glm::vec3 transformedExtents(glm::dot(glm::abs(glm::vec3(linear[0].x, linear[1].x, linear[2].x)), extents),
                                           glm::dot(glm::abs(glm::vec3(linear[0].y, linear[1].y, linear[2].y)), extents),
                                           glm::dot(glm::abs(glm::vec3(linear[0].z, linear[1].z, linear[2].z)), extents));

        return {transformedCenter - transformedExtents, transformedCenter + transformedExtents};
    }
}; // namespace golias
