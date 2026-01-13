#include "core/graphics/mesh.h"

#include "core/engine.h"
#include <stdafx.h>


namespace golias {


    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& v, const std::vector<Uint32>& i)
        : vertex_layout(layout) {
        size_t floats_per_vertex = layout.stride / sizeof(float);
        vertex_count             = floats_per_vertex > 0 ? v.size() / floats_per_vertex : 0;
        index_count              = i.size();
        index_type               = EDataType::UNSIGNED_INT;
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& v) : vertex_layout(layout){
        size_t floats_per_vertex = layout.stride / sizeof(float);
        vertex_count             = floats_per_vertex > 0 ? v.size() / floats_per_vertex : 0;
        index_count              = 0;
        index_type               = EDataType::UNSIGNED_INT;
    }

    std::shared_ptr<Mesh> Mesh::CreateQuad(float width, float height) {


  
        std::vector<float> quad_vertices = {
            // pos(x,y) 
            1.0f,1.0f,
            0.0f,1.0f,
            0.0f,0.0f,
            1.0f,0.0f,
        };

        std::vector<uint32_t> quad_indices = {
            0,1,2,
            0,2,3
        };

        golias::VertexLayout layout;
        layout.elements = {
            {0, 2, EDataType::FLOAT, false, 0},
        };
        layout.stride = 2 * sizeof(float);

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        return rd->CreateMeshFromData(layout, quad_vertices, quad_indices);
    }

    
    std::shared_ptr<Mesh> Mesh::CreateSphere(float radius, uint32_t segments, uint32_t rings) {
        if (segments < 3) {
            segments = 3;
        }
        if (rings < 2) {
            rings = 2;
        }

        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        vertices.reserve((segments + 1) * (rings + 1) * 8);

        for (uint32_t y = 0; y <= rings; ++y) {
            float v   = (float) y / (float) rings;
            float phi = v * glm::pi<float>(); 

            for (uint32_t x = 0; x <= segments; ++x) {
                float u     = (float) x / (float) segments;
                float theta = u * glm::two_pi<float>(); 

                float sx = std::sin(phi) * std::cos(theta);
                float sy = std::cos(phi);  
                float sz = std::sin(phi) * std::sin(theta);

                glm::vec3 pos = glm::vec3(sx, sy, sz) * radius;

                vertices.push_back(pos.x);
                vertices.push_back(pos.y);
                vertices.push_back(pos.z);

                // constant color 
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);

                // uv
                vertices.push_back(u);
                vertices.push_back(1.0f - v);
            }
        }

        uint32_t stride = segments + 1;

        for (uint32_t y = 0; y < rings; ++y) {
            for (uint32_t x = 0; x < segments; ++x) {
                uint32_t i0 = y * stride + x;
                uint32_t i1 = y * stride + x + 1;
                uint32_t i2 = (y + 1) * stride + x;
                uint32_t i3 = (y + 1) * stride + x + 1;

               
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i1);
                indices.push_back(i3);
                indices.push_back(i2);
            }
        }

        golias::VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0},
            {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
            {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)}
        };
        layout.stride = 8 * sizeof(float);

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        return rd->CreateMeshFromData(layout, vertices, indices);
    }

    std::shared_ptr<Mesh> Mesh::CreateBox(const glm::vec3& extents) {
        const auto half_extents = extents * 0.5f;

        // pos(x,y,z) | color(r,g,b) | uv(u,v)
        std::vector<float> box_vertices = {
            // ---------- Front ----------
            -half_extents.x, -half_extents.y,  half_extents.z, 1,1,1, 0,0,
            half_extents.x, -half_extents.y,  half_extents.z, 1,1,1, 1,0,
            half_extents.x,  half_extents.y,  half_extents.z, 1,1,1, 1,1,
            -half_extents.x,  half_extents.y,  half_extents.z, 1,1,1, 0,1,

            // ---------- Back ----------
            -half_extents.x, -half_extents.y, -half_extents.z, 1,1,1, 1,0,
            half_extents.x, -half_extents.y, -half_extents.z, 1,1,1, 0,0,
            half_extents.x,  half_extents.y, -half_extents.z, 1,1,1, 0,1,
            -half_extents.x,  half_extents.y, -half_extents.z, 1,1,1, 1,1,

            // ---------- Left ----------
            -half_extents.x, -half_extents.y, -half_extents.z, 1,1,1, 0,0,
            -half_extents.x, -half_extents.y,  half_extents.z, 1,1,1, 1,0,
            -half_extents.x,  half_extents.y,  half_extents.z, 1,1,1, 1,1,
            -half_extents.x,  half_extents.y, -half_extents.z, 1,1,1, 0,1,

            // ---------- Right ----------
            half_extents.x, -half_extents.y, -half_extents.z, 1,1,1, 1,0,
            half_extents.x, -half_extents.y,  half_extents.z, 1,1,1, 0,0,
            half_extents.x,  half_extents.y,  half_extents.z, 1,1,1, 0,1,
            half_extents.x,  half_extents.y, -half_extents.z, 1,1,1, 1,1,

            // ---------- Top ----------
            -half_extents.x,  half_extents.y, -half_extents.z, 1,1,1, 0,1,
            half_extents.x,  half_extents.y, -half_extents.z, 1,1,1, 1,1,
            half_extents.x,  half_extents.y,  half_extents.z, 1,1,1, 1,0,
            -half_extents.x,  half_extents.y,  half_extents.z, 1,1,1, 0,0,

            // ---------- Bottom ----------
            -half_extents.x, -half_extents.y, -half_extents.z, 1,1,1, 0,0,
            half_extents.x, -half_extents.y, -half_extents.z, 1,1,1, 1,0,
            half_extents.x, -half_extents.y,  half_extents.z, 1,1,1, 1,1,
            -half_extents.x, -half_extents.y,  half_extents.z, 1,1,1, 0,1,
        };

        std::vector<uint32_t> box_indices = {
            0,1,2, 2,3,0,
            5,4,7, 7,6,5,
            8,9,10, 10,11,8,
            13,12,15, 15,14,13,
            19,18,17, 17,16,19,
            20,21,22, 22,23,20
        };

        golias::VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0},
            {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
            {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)}
        };
        layout.stride = 8 * sizeof(float);

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        return rd->CreateMeshFromData(layout, box_vertices, box_indices);
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
