#include "core/graphics/mesh.h"

#include "core/engine.h"
#include <stdafx.h>



namespace golias {



    Mesh::Mesh(const std::string_view path) {

        // std::string extension = FileSystem::GetFileExtension(path);

        // if (extension == "gltf" || extension == "glb") {
        //     if (!LoadGLTF(path, vertex_layout, vertices_data, indices_data, vertex_count, index_count, index_type)) {
        //         spdlog::error("Mesh::Mesh Failed to load model: {} ", path);
        //     }
        // } else if (extension == "obj") {
        //     if (!LoadOBJ(path, vertex_layout, vertices_data, indices_data, vertex_count, index_count, index_type)) {
        //         spdlog::error("Mesh::Mesh Failed to load model: {} ", path);
        //     }
        // } else {
        //     spdlog::error("Mesh::Mesh Unsupported Model format: {} ", path);
        // }
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& v, const std::vector<Uint32>& i)
        : vertex_layout(layout), vertices_data(v), indices_data(i) {
        size_t floats_per_vertex = layout.stride / sizeof(float);
        vertex_count             = floats_per_vertex > 0 ? v.size() / floats_per_vertex : 0;
        index_count              = i.size();
        index_type               = EDataType::UNSIGNED_INT;
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& v) : vertex_layout(layout), vertices_data(v) {
        size_t floats_per_vertex = layout.stride / sizeof(float);
        vertex_count             = floats_per_vertex > 0 ? v.size() / floats_per_vertex : 0;
        index_count              = 0;
        index_type               = EDataType::UNSIGNED_INT;
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
