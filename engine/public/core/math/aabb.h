#pragma once
#include "core/graphics/structs.h"

#include <glm/vec3.hpp>

namespace golias {

    class AABB {
    public:
        AABB() : min(glm::vec3(0.0f)), max(glm::vec3(0.0f)) {
        }


        const glm::vec3& GetMin() const;

        const glm::vec3& GetMax() const;

        void Reset();

        void Expand(const glm::vec3& point);

        glm::vec3 Center() const;
        glm::vec3 Size() const;
        glm::vec3 Extents() const;

        float Radius() const;

        float MaxAxis() const;

    private:
        glm::vec3 min = glm::vec3(0.0f);
        glm::vec3 max = glm::vec3(0.0f);
    };


    AABB ComputeAABBFromInterleavedBuffer(const VertexLayout& layout,
                                            const std::vector<float>& vertices,
                                            const std::vector<uint32_t>& indices);

   

    AABB WorldTransformAABB(const AABB& local, const glm::mat4& model);

} // namespace golias
