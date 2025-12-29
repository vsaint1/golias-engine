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

    std::shared_ptr<Mesh> Mesh::Box() {

        // pos(x,y,z) | color(r,g,b) | uv(u,v)
        std::vector<float> box_vertices = {
            // ---------- Front ----------
            -0.5f,-0.5f, 0.5f, 1,1,1, 0,0,
            0.5f,-0.5f, 0.5f, 1,1,1, 1,0,
            0.5f, 0.5f, 0.5f, 1,1,1, 1,1,
            -0.5f, 0.5f, 0.5f, 1,1,1, 0,1,

            // ---------- Back ----------
            -0.5f,-0.5f,-0.5f, 1,1,1, 1,0,
            0.5f,-0.5f,-0.5f, 1,1,1, 0,0,
            0.5f, 0.5f,-0.5f, 1,1,1, 0,1,
            -0.5f, 0.5f,-0.5f, 1,1,1, 1,1,

            // ---------- Left ----------
            -0.5f,-0.5f,-0.5f, 1,1,1, 0,0,
            -0.5f,-0.5f, 0.5f, 1,1,1, 1,0,
            -0.5f, 0.5f, 0.5f, 1,1,1, 1,1,
            -0.5f, 0.5f,-0.5f, 1,1,1, 0,1,

            // ---------- Right ----------
            0.5f,-0.5f,-0.5f, 1,1,1, 1,0,
            0.5f,-0.5f, 0.5f, 1,1,1, 0,0,
            0.5f, 0.5f, 0.5f, 1,1,1, 0,1,
            0.5f, 0.5f,-0.5f, 1,1,1, 1,1,

            // ---------- Top ----------
            -0.5f, 0.5f,-0.5f, 1,1,1, 0,1,
            0.5f, 0.5f,-0.5f, 1,1,1, 1,1,
            0.5f, 0.5f, 0.5f, 1,1,1, 1,0,
            -0.5f, 0.5f, 0.5f, 1,1,1, 0,0,

            // ---------- Bottom ----------
            -0.5f,-0.5f,-0.5f, 1,1,1, 0,0,
            0.5f,-0.5f,-0.5f, 1,1,1, 1,0,
            0.5f,-0.5f, 0.5f, 1,1,1, 1,1,
            -0.5f,-0.5f, 0.5f, 1,1,1, 0,1,
        };

        std::vector<uint32_t> box_indices = {
            0,  1,  2,  2,  3,  0,  // front
            4,  5,  6,  6,  7,  4,  // back
            8,  9, 10, 10, 11,  8,  // left
            12, 13, 14, 14, 15, 12,  // right
            16, 17, 18, 18, 19, 16,  // top
            20, 21, 22, 22, 23, 20   // bottom
        };

        golias::VertexLayout layout;
        layout.elements = {
            {0, 3, EDataType::FLOAT, false, 0},
            {1, 3, EDataType::FLOAT, false, 3 * sizeof(float)},
            {2, 2, EDataType::FLOAT, false, 6 * sizeof(float)}
        };
        layout.stride = 8 * sizeof(float);

        auto rd = Engine::GetInstance().GetRenderingDevice();
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
