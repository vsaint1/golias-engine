#include "math/frustum.h"

namespace golias {

    Frustum Frustum::FromMatrix(const glm::mat4& clipMatrix) {
        Frustum result;
        const glm::vec4 rows[4] = {glm::vec4(clipMatrix[0][0], clipMatrix[1][0], clipMatrix[2][0], clipMatrix[3][0]),
                                   glm::vec4(clipMatrix[0][1], clipMatrix[1][1], clipMatrix[2][1], clipMatrix[3][1]),
                                   glm::vec4(clipMatrix[0][2], clipMatrix[1][2], clipMatrix[2][2], clipMatrix[3][2]),
                                   glm::vec4(clipMatrix[0][3], clipMatrix[1][3], clipMatrix[2][3], clipMatrix[3][3])};

        // NOTE: Convention engine uses a zero-to-one depth range
        result.mPlanes[0] = rows[3] + rows[0]; // left
        result.mPlanes[1] = rows[3] - rows[0]; // right
        result.mPlanes[2] = rows[3] + rows[1]; // bottom
        result.mPlanes[3] = rows[3] - rows[1]; // top
        result.mPlanes[4] = rows[2]; // near
        result.mPlanes[5] = rows[3] - rows[2]; // far

        for (glm::vec4& plane : result.mPlanes) {
            const float length = glm::length(glm::vec3(plane));
            if (length > std::numeric_limits<float>::epsilon()) {
                plane /= length;
            }
        }

        return result;
    }

    bool Frustum::Intersects(const AABB& bounds) const {
        if (!bounds.IsValid()) {
            return false;
        }

        for (const glm::vec4& plane : mPlanes) {
            const glm::vec3 max = bounds.GetMax();
            const glm::vec3 min = bounds.GetMin();

            // clang-format off
            const glm::vec3 positive(plane.x >= 0.0f ? max.x : min.x,
                                     plane.y >= 0.0f ? max.y : min.y,
                                     plane.z >= 0.0f ? max.z : min.z);
            // clang-format on

            if (glm::dot(glm::vec3(plane), positive) + plane.w < 0.0f) {
                return false;
            }
        }

        return true;
    }
} // namespace golias
