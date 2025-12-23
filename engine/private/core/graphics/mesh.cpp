#include "core/graphics/mesh.h"


namespace golias {


    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<Uint32>& indices) : vertex_layout(layout) {
        vertex_count = vertices.size() / (layout.stride / sizeof(float));
        index_count  = indices.size();

      
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices) : vertex_layout(layout) {
        vertex_count = vertices.size() / (layout.stride / sizeof(float));
        index_count  = 0;

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

}; // namespace golias
