#pragma once

#include "math/aabb.h"

namespace golias {

    class Frustum {
    public:
        static Frustum FromMatrix(const glm::mat4& clipMatrix);

        bool Intersects(const AABB& bounds) const;

    private:
        std::array<glm::vec4, 6> mPlanes = {};
    };

} // namespace golias
