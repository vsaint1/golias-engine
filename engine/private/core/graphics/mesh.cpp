#include "core/graphics/mesh.h"

#include "core/engine.h"
#include <stdafx.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace golias {

    namespace {


        size_t GetDataTypeSize(EDataType type) {
            switch (type) {
            case EDataType::BYTE:
            case EDataType::UNSIGNED_BYTE:
                return 1;
            case EDataType::SHORT:
            case EDataType::UNSIGNED_SHORT:
                return 2;
            case EDataType::INT:
            case EDataType::UNSIGNED_INT:
            case EDataType::FLOAT:
                return 4;
            default:
                return 4;
            }
        }

        EDataType ComponentTypeToDataType(cgltf_component_type type) {
            switch (type) {
            case cgltf_component_type_r_8:
                return EDataType::BYTE;
            case cgltf_component_type_r_8u:
                return EDataType::UNSIGNED_BYTE;
            case cgltf_component_type_r_16:
                return EDataType::SHORT;
            case cgltf_component_type_r_16u:
                return EDataType::UNSIGNED_SHORT;
            case cgltf_component_type_r_32u:
                return EDataType::UNSIGNED_INT;
            case cgltf_component_type_r_32f:
                return EDataType::FLOAT;
            default:
                return EDataType::FLOAT;
            }
        }

        Uint32 GetComponentCount(cgltf_type type) {
            switch (type) {
            case cgltf_type_scalar:
                return 1;
            case cgltf_type_vec2:
                return 2;
            case cgltf_type_vec3:
                return 3;
            case cgltf_type_vec4:
                return 4;
            case cgltf_type_mat2:
                return 4;
            case cgltf_type_mat3:
                return 9;
            case cgltf_type_mat4:
                return 16;
            default:
                return 0;
            }
        }

        bool ExtractFloatData(const cgltf_accessor* accessor, std::vector<float>& out) {
            if (!accessor || !accessor->buffer_view) {
                return false;
            }

            size_t components = GetComponentCount(accessor->type);
            out.resize(accessor->count * components);

            for (size_t i = 0; i < accessor->count; ++i) {
                cgltf_accessor_read_float(accessor, i, &out[i * components], components);
            }

            return true;
        }

        bool ExtractIndexData(const cgltf_accessor* accessor, std::vector<Uint32>& out, EDataType& out_type) {
            if (!accessor || !accessor->buffer_view) {
                return false;
            }

            out_type = ComponentTypeToDataType(accessor->component_type);
            out.resize(accessor->count);

            for (size_t i = 0; i < accessor->count; ++i) {
                out[i] = static_cast<Uint32>(cgltf_accessor_read_index(accessor, i));
            }

            return true;
        }


        bool LoadGLTF(const std::string_view path, VertexLayout& out_layout, std::vector<float>& out_vertices,
                      std::vector<Uint32>& out_indices, size_t& out_vertex_count, size_t& out_index_count, EDataType& out_index_type) {

            auto& fs     = Engine::GetInstance().GetFileSystem();
            auto content = fs.LoadAssetFile(path);
            if (content.empty()) {
                spdlog::error("Failed to load GLTF file: {}", path);
                return false;
            }

            cgltf_options opt{};
            cgltf_data* data{};
            cgltf_result result = cgltf_parse(&opt, content.data(), content.size(), &data);
            if (result != cgltf_result_success) {
                spdlog::error("Failed to parse GLTF: {} (error: {})", path, 0x0);
                return false;
            }

            for (size_t i = 0; i < data->buffers_count; ++i) {
                auto& buf = data->buffers[i];
                if (!buf.uri || strncmp(buf.uri, "data:", 5) == 0) {
                    continue;
                }

                std::string dir(path);
                size_t s = dir.find_last_of("/\\");
                dir      = (s == std::string::npos) ? "" : dir.substr(0, s + 1);
                auto bin = fs.LoadAssetFile(dir + buf.uri);
                if (!bin.empty()) {
                    buf.data = malloc(bin.size());
                    memcpy(buf.data, bin.data(), bin.size());
                    buf.size = bin.size();
                }
            }

            result = cgltf_validate(data);
            if (result != cgltf_result_success) {
                spdlog::warn("GLTF validation warning: {} (error: {})", path, 0x0);
            }

            if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
                spdlog::error("No mesh data found in GLTF: {}", path);

                for (size_t i = 0; i < data->buffers_count; ++i) {
                    if (data->buffers[i].data && data->buffers[i].uri && strncmp(data->buffers[i].uri, "data:", 5) != 0) {
                        free(data->buffers[i].data);
                    }
                }
                cgltf_free(data);
                return false;
            }

            const cgltf_primitive* prim = &data->meshes[0].primitives[0];

            const cgltf_accessor* position_accessor = nullptr;
            const cgltf_accessor* texcoord_accessor = nullptr;
            const cgltf_accessor* color_accessor    = nullptr;
            size_t vertex_count                     = 0;

            for (size_t i = 0; i < prim->attributes_count; ++i) {
                const cgltf_attribute& a = prim->attributes[i];
                if (!a.data) {
                    continue;
                }

                if (a.type == cgltf_attribute_type_position) {
                    position_accessor = a.data;
                    vertex_count      = a.data->count;
                } else if (a.type == cgltf_attribute_type_texcoord && texcoord_accessor == nullptr) {
                    texcoord_accessor = a.data;
                } else if (a.type == cgltf_attribute_type_color && color_accessor == nullptr) {
                    color_accessor = a.data;
                }
            }

            if (vertex_count == 0) {
                spdlog::error("No vertices found in GLTF: {}", path);

                for (size_t i = 0; i < data->buffers_count; ++i) {
                    if (data->buffers[i].data && data->buffers[i].uri && strncmp(data->buffers[i].uri, "data:", 5) != 0) {
                        free(data->buffers[i].data);
                    }
                }

                cgltf_free(data);
                return false;
            }

            out_vertex_count = vertex_count;

            // Extract attribute data
            std::vector<float> positions, texcoords, colors;
            if (position_accessor) {
                ExtractFloatData(position_accessor, positions);
            }
            if (texcoord_accessor) {
                ExtractFloatData(texcoord_accessor, texcoords);
            }
            if (color_accessor) {
                ExtractFloatData(color_accessor, colors);
            }

            out_layout    = VertexLayout();
            Uint32 offset = 0;

            // Position at location 0
            VertexElement pos_el{};
            pos_el.location   = 0;
            pos_el.components = 3;
            pos_el.type       = EDataType::FLOAT;
            pos_el.normalized = false;
            pos_el.offset     = offset;
            offset += 12;
            out_layout.elements.push_back(pos_el);

            // Color at location 1
            VertexElement color_el{};
            color_el.location   = 1;
            color_el.components = 3;
            color_el.type       = EDataType::FLOAT;
            color_el.normalized = false;
            color_el.offset     = offset;
            offset += 12;
            out_layout.elements.push_back(color_el);

            // Texcoord at location 2
            VertexElement texcoord_el{};
            texcoord_el.location   = 2;
            texcoord_el.components = 2;
            texcoord_el.type       = EDataType::FLOAT;
            texcoord_el.normalized = false;
            texcoord_el.offset     = offset;
            offset += 8;
            out_layout.elements.push_back(texcoord_el);

            out_layout.stride = offset;

            out_vertices.clear();
            out_vertices.reserve(vertex_count * 8); // 8 floats per vertex

            for (size_t v = 0; v < vertex_count; ++v) {

                // Position (3 floats)
                if (!positions.empty() && v * 3 + 2 < positions.size()) {
                    out_vertices.push_back(positions[v * 3 + 0]);
                    out_vertices.push_back(positions[v * 3 + 1]);
                    out_vertices.push_back(positions[v * 3 + 2]);
                } else {
                    out_vertices.push_back(0.0f);
                    out_vertices.push_back(0.0f);
                    out_vertices.push_back(0.0f);
                }

                // Color (3 floats) - default white if missing
                if (!colors.empty() && color_accessor) {
                    size_t color_comps = GetComponentCount(color_accessor->type);
                    if (color_comps == 3 && v * 3 + 2 < colors.size()) {
                        out_vertices.push_back(colors[v * 3 + 0]);
                        out_vertices.push_back(colors[v * 3 + 1]);
                        out_vertices.push_back(colors[v * 3 + 2]);
                    } else if (color_comps == 4 && v * 4 + 2 < colors.size()) {
                        // RGBA -> RGB (ignore alpha)
                        out_vertices.push_back(colors[v * 4 + 0]);
                        out_vertices.push_back(colors[v * 4 + 1]);
                        out_vertices.push_back(colors[v * 4 + 2]);
                    } else {
                        out_vertices.push_back(1.0f);
                        out_vertices.push_back(1.0f);
                        out_vertices.push_back(1.0f);
                    }
                } else {
                    // Default white
                    out_vertices.push_back(1.0f);
                    out_vertices.push_back(1.0f);
                    out_vertices.push_back(1.0f);
                }

                // Texcoord (2 floats) - default [0,0] if missing
                if (!texcoords.empty() && texcoord_accessor) {
                    size_t texcoord_comps = GetComponentCount(texcoord_accessor->type);
                    if (texcoord_comps == 2 && v * 2 + 1 < texcoords.size()) {
                        out_vertices.push_back(texcoords[v * 2 + 0]);
                        out_vertices.push_back(texcoords[v * 2 + 1]);
                    } else if (texcoord_comps == 3 && v * 3 + 1 < texcoords.size()) {
                        // vec3 -> vec2 (drop Z)
                        out_vertices.push_back(texcoords[v * 3 + 0]);
                        out_vertices.push_back(texcoords[v * 3 + 1]);
                    } else {
                        out_vertices.push_back(0.0f);
                        out_vertices.push_back(0.0f);
                    }
                } else {
                    // Default UVs
                    out_vertices.push_back(0.0f);
                    out_vertices.push_back(0.0f);
                }
            }

            // Extract indices
            if (prim->indices) {
                if (!ExtractIndexData(prim->indices, out_indices, out_index_type)) {
                    spdlog::error("Failed to extract indices from GLTF: {}", path);
                    out_index_type  = EDataType::UNSIGNED_INT;
                    out_index_count = 0;
                    out_indices.clear();
                } else {
                    out_index_count = out_indices.size();
                }
            } else {
                out_index_type  = EDataType::UNSIGNED_INT;
                out_index_count = 0;
                out_indices.clear();
            }

            for (size_t i = 0; i < data->buffers_count; ++i) {
                if (data->buffers[i].data && data->buffers[i].uri && strncmp(data->buffers[i].uri, "data:", 5) != 0) {
                    free(data->buffers[i].data);
                }
            }

            cgltf_free(data);

            spdlog::info("Successfully parsed GLTF: {} ({} vertices, {} indices)", path, out_vertex_count, out_index_count);
            return true;
        }


        bool LoadOBJ(const std::string_view path, VertexLayout& out_layout, std::vector<float>& out_vertices,
                     std::vector<Uint32>& out_indices, size_t& out_vertex_count, size_t& out_index_count, EDataType& out_index_type) {

            auto& fs = Engine::GetInstance().GetFileSystem();

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string err;

            std::string full_path = fs.GetAssetsPath().append(std::string(path));
            size_t s              = full_path.find_last_of("/\\");
            std::string dir       = (s == std::string::npos) ? "" : full_path.substr(0, s + 1);

            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, full_path.c_str(), dir.c_str(), true);

            if (!err.empty()) {
                if (!ret) {
                    spdlog::error("OBJ error: {}", err);
                } else {
                    spdlog::warn("OBJ warning: {}", err);
                }
            }

            if (!ret) {
                spdlog::error("Failed to parse OBJ file: {}", path);
                return false;
            }

            if (shapes.empty()) {
                spdlog::error("No shapes found in OBJ: {}", path);
                return false;
            }

            // Setup vertex layout: Position(3) + Color(3) + TexCoord(2)
            out_layout    = VertexLayout();
            Uint32 offset = 0;

            // Position at location 0
            VertexElement pos_el{};
            pos_el.location   = 0;
            pos_el.components = 3;
            pos_el.type       = EDataType::FLOAT;
            pos_el.normalized = false;
            pos_el.offset     = offset;
            offset += 12;
            out_layout.elements.push_back(pos_el);

            // Color at location 1
            VertexElement color_el{};
            color_el.location   = 1;
            color_el.components = 3;
            color_el.type       = EDataType::FLOAT;
            color_el.normalized = false;
            color_el.offset     = offset;
            offset += 12;
            out_layout.elements.push_back(color_el);

            // Texcoord at location 2
            VertexElement texcoord_el{};
            texcoord_el.location   = 2;
            texcoord_el.components = 2;
            texcoord_el.type       = EDataType::FLOAT;
            texcoord_el.normalized = false;
            texcoord_el.offset     = offset;
            offset += 8;
            out_layout.elements.push_back(texcoord_el);

            out_layout.stride = offset;

            // Combine all shapes into one mesh
            out_vertices.clear();
            out_indices.clear();

            size_t vertex_offset = 0;

            for (const auto& shape : shapes) {
                const auto& mesh = shape.mesh;

                // Process each face (already triangulated)
                size_t index_offset = 0;
                for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
                    size_t fv = mesh.num_face_vertices[f];

                    // Should be triangles due to triangulation
                    for (size_t v = 0; v < fv; ++v) {
                        tinyobj::index_t idx = mesh.indices[index_offset + v];

                        // Position (required)
                        if (idx.vertex_index >= 0 && 3 * idx.vertex_index + 2 < attrib.vertices.size()) {
                            out_vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                            out_vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                            out_vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                        } else {
                            out_vertices.push_back(0.0f);
                            out_vertices.push_back(0.0f);
                            out_vertices.push_back(0.0f);
                        }

                        // Color (default white - OBJ doesn't typically have vertex colors)
                        out_vertices.push_back(1.0f);
                        out_vertices.push_back(1.0f);
                        out_vertices.push_back(1.0f);

                        // Texcoord (optional)
                        if (idx.texcoord_index >= 0 && 2 * idx.texcoord_index + 1 < attrib.texcoords.size()) {
                            out_vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                            out_vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                        } else {
                            out_vertices.push_back(0.0f);
                            out_vertices.push_back(0.0f);
                        }
                    }

                    // Generate indices
                    for (size_t v = 0; v < fv; ++v) {
                        out_indices.push_back(static_cast<Uint32>(vertex_offset + v));
                    }

                    vertex_offset += fv;
                    index_offset += fv;
                }
            }

            out_vertex_count = vertex_offset;
            out_index_count  = out_indices.size();
            out_index_type   = EDataType::UNSIGNED_INT;

            spdlog::info("Successfully parsed OBJ: {} ({} vertices, {} indices)", path, out_vertex_count, out_index_count);
            return true;
        }


    } // namespace


    Mesh::Mesh(const std::string_view path) {

        std::string extension = FileSystem::GetFileExtension(path);

        if (extension == "gltf" || extension == "glb") {
            if (!LoadGLTF(path, vertex_layout, vertices_data, indices_data, vertex_count, index_count, index_type)) {
                spdlog::error("Mesh::Mesh Failed to load model: {} ", path);
            }
        } else if (extension == "obj") {
            if (!LoadOBJ(path, vertex_layout, vertices_data, indices_data, vertex_count, index_count, index_type)) {
                spdlog::error("Mesh::Mesh Failed to load model: {} ", path);
            }
        } else {
            spdlog::error("Mesh::Mesh Unsupported Model format: {} ", path);
        }
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
