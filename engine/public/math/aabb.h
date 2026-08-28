#pragma once

#include "stdafx.h"

namespace golias {

    class AABB {

    public:
        AABB() = default;

        AABB(const glm::vec3& min, const glm::vec3& max) : mMin(min), mMax(max) {
        }

        bool IsValid() const;

        void Expand(const glm::vec3& point);

        glm::vec3 GetMin() const;

        glm::vec3 GetMax() const;

        glm::vec3 GetCenter() const;

        glm::vec3 GetExtents() const;

        AABB Transformed(const glm::mat4& transform) const;

    private:
        glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 mMax = glm::vec3(std::numeric_limits<float>::lowest());
    };

} // namespace golias
