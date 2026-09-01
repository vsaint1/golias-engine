#pragma once

#include "math/tangent.h"

namespace golias {


    void GenerateTangents(std::vector<float>& vertices,
                          const std::vector<uint32_t>& indices,
                          const TangentLayout& layout,
                          size_t vertexCount) {

        if (layout.Stride == 0 || vertices.empty()) {
            return;
        }

        if (vertexCount == 0) {
            vertexCount = vertices.size() / layout.Stride;
        }

        auto at = [&](size_t vertex, size_t offset) -> float& { return vertices[vertex * layout.Stride + offset]; };

        std::vector<glm::vec3> tangents(vertexCount, glm::vec3(0.0f));
        std::vector<glm::vec3> bitangents(vertexCount, glm::vec3(0.0f));
        std::vector<uint32_t> weight(vertexCount, 0);

        for (size_t tri = 0; tri + 2 < indices.size(); tri += 3) {
            const uint32_t i0 = indices[tri];
            const uint32_t i1 = indices[tri + 1];
            const uint32_t i2 = indices[tri + 2];

            if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) {
                continue;
            }

            const glm::vec3 p0(at(i0, layout.Position), at(i0, layout.Position + 1), at(i0, layout.Position + 2));
            const glm::vec3 p1(at(i1, layout.Position), at(i1, layout.Position + 1), at(i1, layout.Position + 2));
            const glm::vec3 p2(at(i2, layout.Position), at(i2, layout.Position + 1), at(i2, layout.Position + 2));

            const glm::vec2 uv0(at(i0, layout.TexCoord), at(i0, layout.TexCoord + 1));
            const glm::vec2 uv1(at(i1, layout.TexCoord), at(i1, layout.TexCoord + 1));
            const glm::vec2 uv2(at(i2, layout.TexCoord), at(i2, layout.TexCoord + 1));

            const glm::vec3 edge1    = p1 - p0;
            const glm::vec3 edge2    = p2 - p0;
            const glm::vec2 deltaUV1 = uv1 - uv0;
            const glm::vec2 deltaUV2 = uv2 - uv0;

            const float denominator = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
            if (std::abs(denominator) < 1e-8f) {
                continue;
            }

            const float r             = 1.0f / denominator;
            const glm::vec3 tangent   = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;
            const glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r;

            tangents[i0] += tangent;
            tangents[i1] += tangent;
            tangents[i2] += tangent;
            bitangents[i0] += bitangent;
            bitangents[i1] += bitangent;
            bitangents[i2] += bitangent;
            weight[i0]++;
            weight[i1]++;
            weight[i2]++;
        }

        for (size_t i = 0; i < vertexCount; ++i) {

            // Skip vertices which already have tangents.
            const glm::vec3 existing(at(i, layout.Tangent), at(i, layout.Tangent + 1), at(i, layout.Tangent + 2));
            if (glm::length2(existing) > 1e-6f) {
                continue;
            }

            if (weight[i] == 0) {
                continue;
            }

            glm::vec3 normal(at(i, layout.Normal), at(i, layout.Normal + 1), at(i, layout.Normal + 2));
            normal = glm::normalize(normal);

            glm::vec3 tangent = tangents[i];
            tangent -= normal * glm::dot(normal, tangent);
            tangent = glm::normalize(tangent);

            glm::vec3 bitangent = bitangents[i];
            bitangent -= normal * glm::dot(normal, bitangent);
            bitangent -= tangent * glm::dot(tangent, bitangent);
            bitangent = glm::normalize(bitangent);

            at(i, layout.Tangent)       = tangent.x;
            at(i, layout.Tangent + 1)   = tangent.y;
            at(i, layout.Tangent + 2)   = tangent.z;
            at(i, layout.Bitangent)     = bitangent.x;
            at(i, layout.Bitangent + 1) = bitangent.y;
            at(i, layout.Bitangent + 2) = bitangent.z;
        }
    }

} // namespace golias
