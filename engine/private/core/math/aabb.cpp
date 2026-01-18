#include "core/math/aabb.h"

#include <limits>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace golias {
    const glm::vec3& AABB::GetMin() const {
        return min;
    }

    const glm::vec3& AABB::GetMax() const {
        return max;
    }

    void AABB::Reset() {
        min = glm::vec3(std::numeric_limits<float>::max());
        max = glm::vec3(-std::numeric_limits<float>::max());
    }

    void AABB::Expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    glm::vec3 AABB::Center() const {
        return (min + max) * 0.5f;
    }

    glm::vec3 AABB::Size() const {
        return max - min;
    }

    float AABB::Radius() const {
        return glm::length(Extents());
    }

    float AABB::MaxAxis() const {
        glm::vec3 s = Size();
        return glm::max(s.x, glm::max(s.y, s.z));
    }

    glm::vec3 AABB::Extents() const {
        return Size() * 0.5f;
    }


    AABB ComputeAABBFromInterleavedBuffer(const VertexLayout& layout,
                                           const std::vector<float>& vertices,
                                           const std::vector<uint32_t>& indices) {

        AABB aabb;
        aabb.Reset();

        const VertexElement* posElem = nullptr;
        for (const auto& e : layout.elements) {
            if (e.location == VertexElement::POSITION_INDEX) {
                posElem = &e;
                break;
            }
        }

        if (!posElem || posElem->components < 3) {
            return aabb;
        }

        const uint32_t strideFloats    = layout.stride / sizeof(float);
        const uint32_t posOffsetFloats = posElem->offset / sizeof(float);

        auto expandVertex = [&](uint32_t vertexIndex) {
            const float* base = &vertices[vertexIndex * strideFloats + posOffsetFloats];

            glm::vec3 pos(base[0], base[1], base[2]);
            aabb.Expand(pos);
        };

        if (!indices.empty()) {
            for (uint32_t idx : indices) {
                expandVertex(idx);
            }
        } else {
            const uint32_t vertexCount = vertices.size() / strideFloats;
            for (uint32_t i = 0; i < vertexCount; ++i) {
                expandVertex(i);
            }
        }

        return aabb;
    }


    AABB WorldTransformAABB(const AABB& local, const glm::mat4& model) {
        auto min = local.GetMin();
        auto max = local.GetMax();

        glm::vec3 corners[8] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {max.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z},
        };

        AABB world;
        world.Reset();

        for (auto& c : corners) {
            glm::vec3 p = glm::vec3(model * glm::vec4(c, 1.0f));
            world.Expand(p);
        }

        return world;
    }
} // namespace golias
