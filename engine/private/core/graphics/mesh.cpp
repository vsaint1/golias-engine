#include "core/graphics/mesh.h"

#include "core/engine.h"
#include <stdafx.h>


namespace golias {


    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& v, const std::vector<Uint32>& i) : vertex_layout(layout) {
        size_t floats_per_vertex = layout.stride / sizeof(float);
        vertex_count             = floats_per_vertex > 0 ? v.size() / floats_per_vertex : 0;
        index_count              = i.size();
        index_type               = EDataType::UNSIGNED_INT;
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& v) : vertex_layout(layout) {
        size_t floats_per_vertex = layout.stride / sizeof(float);
        vertex_count             = floats_per_vertex > 0 ? v.size() / floats_per_vertex : 0;
        index_count              = 0;
        index_type               = EDataType::UNSIGNED_INT;
    }

    std::shared_ptr<Mesh> Mesh::CreateQuad(float width, float height) {


        std::vector<float> quad_vertices = {
            // pos(x,y)
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
        };

        std::vector<uint32_t> quad_indices = {0, 1, 2, 0, 2, 3};

        golias::VertexLayout layout;
        layout.elements = {
            {0, 2, EDataType::FLOAT, false, 0},
        };
        layout.stride = 2 * sizeof(float);

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        return rd->CreateMeshFromData(layout, quad_vertices, quad_indices);
    }


    std::shared_ptr<Mesh> Mesh::CreateSphere(float radius, uint32_t segments, uint32_t rings) {
        segments = glm::max(segments, 3u);
        rings    = glm::max(rings, 2u);

        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t y = 0; y <= rings; ++y) {
            float v   = (float) y / rings;
            float phi = v * glm::pi<float>();

            for (uint32_t x = 0; x <= segments; ++x) {
                float u     = (float) x / segments;
                float theta = u * glm::two_pi<float>();

                glm::vec3 normal = {sin(phi) * cos(theta), cos(phi), sin(phi) * sin(theta)};

                glm::vec3 pos = normal * radius;

                // pos
                vertices.push_back(pos.x);
                vertices.push_back(pos.y);
                vertices.push_back(pos.z);

                // color
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);

                // uv
                vertices.push_back(u);
                vertices.push_back(1.0f - v);

                // normal
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
            }
        }

        uint32_t stride = segments + 1;
        for (uint32_t y = 0; y < rings; ++y) {
            for (uint32_t x = 0; x < segments; ++x) {
                uint32_t i0 = y * stride + x;
                uint32_t i1 = i0 + 1;
                uint32_t i2 = i0 + stride;
                uint32_t i3 = i2 + 1;

                indices.insert(indices.end(), {i0, i1, i2, i1, i3, i2});
            }
        }

        VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0                },
            {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
            {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)},
            {3, 3, EDataType::FLOAT, false, 8 * sizeof(float)}
        };
        layout.stride = 11 * sizeof(float);

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        return rd->CreateMeshFromData(layout, vertices, indices);
    }

    std::shared_ptr<Mesh> Mesh::CreateBox(const glm::vec3& extents) {
        glm::vec3 h = extents * 0.5f;

        struct Face {
            glm::vec3 n;
            glm::vec3 v[4];
        };

        Face faces[] = {
            {{ 0, 0, 1}, {{-h.x,-h.y, h.z},{ h.x,-h.y, h.z},{ h.x, h.y, h.z},{-h.x, h.y, h.z}}},
            {{ 0, 0,-1}, {{ h.x,-h.y,-h.z},{-h.x,-h.y,-h.z},{-h.x, h.y,-h.z},{ h.x, h.y,-h.z}}},
            {{-1, 0, 0}, {{-h.x,-h.y,-h.z},{-h.x,-h.y, h.z},{-h.x, h.y, h.z},{-h.x, h.y,-h.z}}},
            {{ 1, 0, 0}, {{ h.x,-h.y, h.z},{ h.x,-h.y,-h.z},{ h.x, h.y,-h.z},{ h.x, h.y, h.z}}},
            {{ 0, 1, 0}, {{-h.x, h.y, h.z},{ h.x, h.y, h.z},{ h.x, h.y,-h.z},{-h.x, h.y,-h.z}}},
            {{ 0,-1, 0}, {{-h.x,-h.y,-h.z},{ h.x,-h.y,-h.z},{ h.x,-h.y, h.z},{-h.x,-h.y, h.z}}},
        };

        std::vector<float> vertices;
        std::vector<uint32_t> indices;
        uint32_t index = 0;

        for (auto& f : faces) {
            for (int i = 0; i < 4; ++i) {
                auto& p = f.v[i];
                auto& n = f.n;

                vertices.insert(vertices.end(), {
                    p.x,p.y,p.z,
                    1,1,1,
                    (i==1||i==2)?1.0f:0.0f,
                    (i>=2)?1.0f:0.0f,
                    n.x,n.y,n.z
                });
            }

            indices.insert(indices.end(), {
                index, index+1, index+2,
                index, index+2, index+3
            });

            index += 4;
        }

        VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0},
            {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
            {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)},
            {3, 3, EDataType::FLOAT, false, 8 * sizeof(float)}
        };
        layout.stride = 11 * sizeof(float);

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        return rd->CreateMeshFromData(layout, vertices, indices);
    }
   


    const VertexLayout& Mesh::GetVertexLayout() const {
        return vertex_layout;
    }

    void Mesh::SetVertexLayout(const VertexLayout& layout) {
        vertex_layout = layout;
    }

    void Mesh::SetVertexCount(size_t count) {
        vertex_count = count;
    }

    size_t Mesh::GetVertexCount() const {
        return vertex_count;
    }

    void Mesh::SetIndexCount(size_t count) {
        index_count = count;
    }

    size_t Mesh::GetIndexCount() const {
        return index_count;
    }

    EDataType Mesh::GetIndexType() const {
        return index_type;
    }

    void Mesh::SetIndexType(EDataType type) {
        index_type = type;
    }

} // namespace golias
