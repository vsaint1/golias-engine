#include "scene/game_object.h"

#include "core/engine.h"
#include "core/graphics/structs.h"
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define TINYOBJLOADER_IMPLEMENTATION

#include "core/graphics/material.h"
#include "scene/3d/mesh_component.h"
#include <tiny_obj_loader.h>

#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

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


    void GameObject::AddComponent(Component* pComponent) {
        components.emplace_back(pComponent);
        pComponent->SetOwner(this);

        spdlog::info("GameObject::AddComponent added Component: {} to GameObject: {}", typeid(*pComponent).name(), typeid(*this).name());
    }

    void GameObject::Update(float deltaTime) {
        for (auto& component : components) {
            component->Update(deltaTime);
        }

        for (auto it = children.begin(); it != children.end();) {
            if ((*it)->IsAlive()) {
                (*it)->Update(deltaTime);
                ++it; 
            } else {
                it = children.erase(it); 
            }
        }
    }

    const std::string& GameObject::GetName() const {
        return name;
    }

    void GameObject::SetName(const std::string& newName) {
        name = newName;
    }

    bool GameObject::SetParent(GameObject* pParent) {

        if (!scene) {
            return false;
        }

        return scene->SetParent(this, pParent);
    }

    Scene* GameObject::GetScene() const {
        return scene;
    }


    GameObject* GameObject::GetParent() const {
        return parent;
    }


    const std::vector<std::unique_ptr<GameObject>>& GameObject::GetChildren() const {
        return children;
    }

    void GameObject::Destroy() {
        is_alive = false;
    }

    bool GameObject::IsAlive() const {
        return is_alive;
    }


    glm::vec3& GameObject::GetPosition() {
        return position;
    }

    void GameObject::SetPosition(const glm::vec3& newPosition) {
        position = newPosition;
    }

    glm::quat& GameObject::GetRotation() {
        return rotation;
    }

    void GameObject::SetRotation(const glm::quat& newRotation) {
        rotation = newRotation;
    }

    glm::vec3& GameObject::GetScale() {
        return scale;
    }

    void GameObject::SetScale(const glm::vec3& newScale) {
        scale = newScale;
    }

    glm::mat4 GameObject::GetLocalTransform() const {
        glm::mat4 mat = glm::mat4(1.0f);
        mat           = glm::translate(mat, position);
        mat           = mat * glm::mat4_cast(rotation);
        mat           = glm::scale(mat, scale);

        return mat;
    }

    glm::mat4 GameObject::GetWorldTransform() const {
        if (parent) {
            return parent->GetWorldTransform() * GetLocalTransform();
        } else {
            return GetLocalTransform();
        }
    }

    glm::vec3 GameObject::GetWorldPosition() const {
        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return glm::vec3(hom) / hom.w;
    }

    void ParseGLTFNode(cgltf_node* node, GameObject* parent, cgltf_data* data, const std::string& base_path);
    void ParseOBJ(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes,
                        const std::vector<tinyobj::material_t>& materials, GameObject* parent, const std::string& base_path);

 
    void CreateMeshComponentGLTF(GameObject* gameObject, const VertexLayout& layout, const std::vector<float>& vertices,
                             const std::vector<Uint32>& indices, cgltf_material* gltf_material, const std::string& base_path) {
        auto& engine = Engine::GetInstance();
        auto rd      = engine.GetRenderingDevice();

      
        std::shared_ptr<Mesh> mesh = rd->CreateMeshFromData(layout, vertices, indices);

       
        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
        material->SetShader(shader);

        // Default values
        glm::vec4 base_color(1.0f);
        float metallic  = 0.0f;
        float roughness = 1.0f;
        glm::vec3 emissive(0.0f);

        // Texture flags (default to 0 - no texture)
        material->SetParameter("HAS_ALBEDO", 0);
        material->SetParameter("HAS_METALLIC", 0);
        material->SetParameter("HAS_ROUGHNESS", 0);
        material->SetParameter("HAS_NORMAL", 0);
        material->SetParameter("HAS_AO", 0);
        material->SetParameter("HAS_EMISSIVE", 0);

        if (gltf_material && gltf_material->has_pbr_metallic_roughness) {
            auto& pbr = gltf_material->pbr_metallic_roughness;

            // Base color factor
            base_color = glm::vec4(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3]);

            // Albedo texture
            if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image && pbr.base_color_texture.texture->image->uri) {
                std::string tex_path = base_path + pbr.base_color_texture.texture->image->uri;
                auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);
                if (texture) {
                    material->SetParameter("ALBEDO_TEXTURE", texture);
                    material->SetParameter("HAS_ALBEDO", 1);
                }
            }

            // Metallic-Roughness texture (single texture with metallic in B, roughness in G)
            metallic  = pbr.metallic_factor;
            roughness = pbr.roughness_factor;

            if (pbr.metallic_roughness_texture.texture && pbr.metallic_roughness_texture.texture->image
                && pbr.metallic_roughness_texture.texture->image->uri) {
                std::string tex_path = base_path + pbr.metallic_roughness_texture.texture->image->uri;
                auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);
                if (texture) {
                    // In GLTF, metallic is in B channel, roughness is in G channel
                    material->SetParameter("METALLIC_TEXTURE", texture);
                    material->SetParameter("ROUGHNESS_TEXTURE", texture);
                    material->SetParameter("HAS_METALLIC", 1);
                    material->SetParameter("HAS_ROUGHNESS", 1);
                }
            }
        }

        // Normal map
        if (gltf_material && gltf_material->normal_texture.texture && gltf_material->normal_texture.texture->image
            && gltf_material->normal_texture.texture->image->uri) {
            std::string tex_path = base_path + gltf_material->normal_texture.texture->image->uri;
            auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);
            if (texture) {
                material->SetParameter("NORMAL_TEXTURE", texture);
                material->SetParameter("HAS_NORMAL", 1);
            }
        }

        // Occlusion map
        if (gltf_material && gltf_material->occlusion_texture.texture && gltf_material->occlusion_texture.texture->image
            && gltf_material->occlusion_texture.texture->image->uri) {
            std::string tex_path = base_path + gltf_material->occlusion_texture.texture->image->uri;
            auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);

            if (texture) {
                material->SetParameter("AO_TEXTURE", texture);
                material->SetParameter("HAS_AO", 1);
            }
        }

        // Emissive
        if (gltf_material && gltf_material->has_emissive_strength) {
            emissive = glm::vec3(gltf_material->emissive_factor[0], gltf_material->emissive_factor[1], gltf_material->emissive_factor[2]);

            if (gltf_material->emissive_texture.texture && gltf_material->emissive_texture.texture->image
                && gltf_material->emissive_texture.texture->image->uri) {
                std::string tex_path = base_path + gltf_material->emissive_texture.texture->image->uri;
                auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);

                if (texture) {
                    material->SetParameter("EMISSIVE_TEXTURE", texture);
                    material->SetParameter("HAS_EMISSIVE", 1);
                }
            }
        }

        material->SetParameter("BASE_COLOR", base_color);
        material->SetParameter("METALLIC_FACTOR", metallic);
        material->SetParameter("ROUGHNESS_FACTOR", roughness);
        material->SetParameter("EMISSIVE_FACTOR", emissive);

        gameObject->AddComponent(new MeshComponent(mesh, material));
    }

   
    void CreateMeshComponentOBJ(GameObject* gameObject, const VertexLayout& layout, const std::vector<float>& vertices,
                                const std::vector<Uint32>& indices, const tinyobj::material_t* obj_material, const std::string& base_path) {
        auto& engine = Engine::GetInstance();
        auto rd      = engine.GetRenderingDevice();

       
        std::shared_ptr<Mesh> mesh = rd->CreateMeshFromData(layout, vertices, indices);

 
        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
        material->SetShader(shader);

        // Default texture flags
        material->SetParameter("HAS_ALBEDO", 0);
        material->SetParameter("HAS_METALLIC", 0);
        material->SetParameter("HAS_ROUGHNESS", 0);
        material->SetParameter("HAS_NORMAL", 0);
        material->SetParameter("HAS_AO", 0);
        material->SetParameter("HAS_EMISSIVE", 0);

        glm::vec4 base_color(1.0f);

        if (obj_material) {
            // Diffuse color
            base_color = glm::vec4(obj_material->diffuse[0], obj_material->diffuse[1], obj_material->diffuse[2], 1.0f);

            // Diffuse texture
            if (!obj_material->diffuse_texname.empty()) {
                std::string tex_path = base_path + obj_material->diffuse_texname;
                auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);

                if (texture) {
                    material->SetParameter("ALBEDO_TEXTURE", texture);
                    material->SetParameter("HAS_ALBEDO", 1);
                }
            }

            // Normal map
            if (!obj_material->normal_texname.empty()) {
                std::string tex_path = base_path + obj_material->normal_texname;
                auto texture         = Engine::GetInstance().GetTextureManager2D().EnsureTexture(tex_path);
                if (texture) {
                    material->SetParameter("NORMAL_TEXTURE", texture);
                    material->SetParameter("HAS_NORMAL", 1);
                }
            }

            float roughness = 1.0f - (obj_material->shininess / 1000.0f);
            material->SetParameter("ROUGHNESS_FACTOR", glm::clamp(roughness, 0.0f, 1.0f));
        }

        material->SetParameter("BASE_COLOR", base_color);
        material->SetParameter("METALLIC_FACTOR", 0.0f);

       
        gameObject->AddComponent(new MeshComponent(mesh, material));
    }

    void ParseGLTFNode(cgltf_node* node, GameObject* parent, cgltf_data* data, const std::string& base_path) {
        auto& engine = Engine::GetInstance();
        auto scene   = engine.GetScene();

        std::string node_name  = node->name ? node->name : "Node";
        GameObject* nodeObject = scene->CreateObject(node_name, parent);

        if (node->has_matrix) {
            auto mat = glm::make_mat4(node->matrix);

            glm::vec3 scale, translation, skew;
            glm::quat rotation;
            glm::vec4 perspective;
            glm::decompose(mat, scale, rotation, translation, skew, perspective);

            nodeObject->SetPosition(translation);
            nodeObject->SetRotation(rotation);
            nodeObject->SetScale(scale);
        } else {
            if (node->has_translation) {
                nodeObject->SetPosition(glm::vec3(node->translation[0], node->translation[1], node->translation[2]));
            }
            if (node->has_rotation) {
                nodeObject->SetRotation(glm::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
            }
            if (node->has_scale) {
                nodeObject->SetScale(glm::vec3(node->scale[0], node->scale[1], node->scale[2]));
            }
        }

        if (node->mesh) {
            cgltf_mesh* mesh_data = node->mesh;

            for (cgltf_size p = 0; p < mesh_data->primitives_count; ++p) {
                cgltf_primitive* prim = &mesh_data->primitives[p];

                const cgltf_accessor* position_accessor = nullptr;
                const cgltf_accessor* texcoord_accessor = nullptr;
                const cgltf_accessor* color_accessor    = nullptr;
                size_t vertex_count                     = 0;

                for (cgltf_size a = 0; a < prim->attributes_count; ++a) {
                    const cgltf_attribute& attr = prim->attributes[a];
                    switch (attr.type) {
                    case cgltf_attribute_type_position:
                        position_accessor = attr.data;
                        vertex_count      = attr.data->count;
                        break;
                    case cgltf_attribute_type_texcoord:
                        if (!texcoord_accessor) {
                            texcoord_accessor = attr.data;
                        }
                        break;
                    case cgltf_attribute_type_color:
                        if (!color_accessor) {
                            color_accessor = attr.data;
                        }
                        break;
                    default:
                        break;
                    }
                }

                if (!position_accessor || vertex_count == 0) {
                    continue;
                }

                // Setup vertex layout: Position(3) + Color(3) + TexCoord(2)
                VertexLayout layout;
                Uint32 offset = 0;

                layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
                offset += 12; // Position
                layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
                offset += 12; // Color
                layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
                offset += 8; // TexCoord
                layout.stride = offset;

                // Extract attribute data
                std::vector<float> positions, texcoords, colors;
                ExtractFloatData(position_accessor, positions);
                if (texcoord_accessor) {
                    ExtractFloatData(texcoord_accessor, texcoords);
                }
                if (color_accessor) {
                    ExtractFloatData(color_accessor, colors);
                }

                // Interleave vertex data
                std::vector<float> vertices;
                vertices.reserve(vertex_count * 8);

                for (size_t v = 0; v < vertex_count; ++v) {
                    // Position
                    if (v * 3 + 2 < positions.size()) {
                        vertices.push_back(positions[v * 3 + 0]);
                        vertices.push_back(positions[v * 3 + 1]);
                        vertices.push_back(positions[v * 3 + 2]);
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});
                    }

                    // Color
                    if (!colors.empty() && color_accessor) {
                        size_t comps = GetComponentCount(color_accessor->type);
                        if (comps >= 3 && v * comps + 2 < colors.size()) {
                            vertices.push_back(colors[v * comps + 0]);
                            vertices.push_back(colors[v * comps + 1]);
                            vertices.push_back(colors[v * comps + 2]);
                        } else {
                            vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});
                        }
                    } else {
                        vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});
                    }

                    // TexCoord
                    if (!texcoords.empty() && texcoord_accessor) {
                        size_t comps = GetComponentCount(texcoord_accessor->type);
                        if (comps >= 2 && v * comps + 1 < texcoords.size()) {
                            vertices.push_back(texcoords[v * comps + 0]);
                            vertices.push_back(texcoords[v * comps + 1]);
                        } else {
                            vertices.insert(vertices.end(), {0.0f, 0.0f});
                        }
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 0.0f});
                    }
                }

                // Extract indices
                std::vector<Uint32> indices;
                EDataType index_type;
                if (prim->indices) {
                    ExtractIndexData(prim->indices, indices, index_type);
                }

                CreateMeshComponentGLTF(nodeObject, layout, vertices, indices, prim->material, base_path);
            }
        }

        for (cgltf_size c = 0; c < node->children_count; ++c) {
            ParseGLTFNode(node->children[c], nodeObject, data, base_path);
        }
    }

    void ParseOBJ(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes,
                        const std::vector<tinyobj::material_t>& materials, GameObject* rootObject, const std::string& base_path) {
        auto& engine = Engine::GetInstance();
        auto scene   = engine.GetScene();

        for (const auto& shape : shapes) {
            GameObject* shapeObject = scene->CreateObject(shape.name.empty() ? "Shape" : shape.name, rootObject);

            const auto& mesh = shape.mesh;

            VertexLayout layout;
            Uint32 offset = 0;
            layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
            offset += 12; // Position
            layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
            offset += 12; // Color
            layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
            offset += 8; // TexCoord
            layout.stride = offset;

            std::vector<float> vertices;
            std::vector<Uint32> indices;
            size_t vertex_offset = 0;

            // Process each face
            size_t index_offset = 0;
            for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
                size_t fv = mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; ++v) {
                    tinyobj::index_t idx = mesh.indices[index_offset + v];

                    // Position
                    if (idx.vertex_index >= 0 && 3 * idx.vertex_index + 2 < attrib.vertices.size()) {
                        vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                        vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                        vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});
                    }

                    // Color (white default)
                    vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});

                    // TexCoord
                    if (idx.texcoord_index >= 0 && 2 * idx.texcoord_index + 1 < attrib.texcoords.size()) {
                        vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                        vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 0.0f});
                    }
                }

                // Generate indices
                for (size_t v = 0; v < fv; ++v) {
                    indices.push_back(static_cast<Uint32>(vertex_offset + v));
                }

                vertex_offset += fv;
                index_offset += fv;
            }

            // Get material pointer
            const tinyobj::material_t* mat_ptr = nullptr;
            if (!mesh.material_ids.empty() && mesh.material_ids[0] >= 0 && mesh.material_ids[0] < materials.size()) {
                mat_ptr = &materials[mesh.material_ids[0]];
            }

            CreateMeshComponentOBJ(shapeObject, layout, vertices, indices, mat_ptr, base_path);
        }
    }

    GameObject* GameObject::LoadModel(const std::string_view pPath) {
        auto& engine = Engine::GetInstance();
        auto& fs     = engine.GetFileSystem();

        std::string extension = FileSystem::GetFileExtension(pPath);
        std::string dir(pPath);
        size_t s = dir.find_last_of("/\\");
        dir      = (s == std::string::npos) ? "" : dir.substr(0, s + 1);

        std::string model_name(pPath);
        size_t last_slash = model_name.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            model_name = model_name.substr(last_slash + 1);
        }

        GameObject* rootObject = engine.GetScene()->CreateObject(model_name, nullptr);

        if (extension == "gltf" || extension == "glb") {
            auto contents = fs.LoadAssetFileText(pPath);
            if (contents.empty()) {
                spdlog::error("Failed to load GLTF file: {}", pPath);
                rootObject->Destroy();
                return nullptr;
            }

            cgltf_options options = {};
            cgltf_data* data      = nullptr;

            cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data);
            if (res != cgltf_result_success) {
                spdlog::error("Failed to parse GLTF: {}", pPath);
                rootObject->Destroy();
                return nullptr;
            }

            for (size_t i = 0; i < data->buffers_count; ++i) {
                auto& buf = data->buffers[i];
                if (!buf.uri || strncmp(buf.uri, "data:", 5) == 0) {
                    continue;
                }

                auto bin = fs.LoadAssetFile(dir + buf.uri);
                if (!bin.empty()) {
                    buf.data = malloc(bin.size());
                    memcpy(buf.data, bin.data(), bin.size());
                    buf.size = bin.size();
                }
            }

            res = cgltf_validate(data);
            if (res != cgltf_result_success) {
                spdlog::warn("GLTF validation warning: {}", pPath);
            }

            cgltf_scene* scene = data->scene ? data->scene : &data->scenes[0];
            for (cgltf_size i = 0; i < scene->nodes_count; ++i) {
                ParseGLTFNode(scene->nodes[i], rootObject, data, dir);
            }

            for (size_t i = 0; i < data->buffers_count; ++i) {
                if (data->buffers[i].data && data->buffers[i].uri && strncmp(data->buffers[i].uri, "data:", 5) != 0) {
                    free(data->buffers[i].data);
                }
            }

            cgltf_free(data);

            spdlog::info("Successfully loaded GLTF model: {}", pPath);

        } else if (extension == "obj") {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string err;

            std::string full_path = fs.GetAssetsPath() + std::string(pPath);
            std::string mtl_dir   = fs.GetAssetsPath() + dir;

            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, full_path.c_str(), mtl_dir.c_str(), true);

            if (!err.empty()) {
                if (!ret) {
                    spdlog::error("OBJ error: {}", err);
                    rootObject->Destroy();
                    return nullptr;
                } else {
                    spdlog::warn("OBJ warning: {}", err);
                }
            }

            if (shapes.empty()) {
                spdlog::error("No shapes found in OBJ: {}", pPath);
                rootObject->Destroy();
                return nullptr;
            }

            ParseOBJ(attrib, shapes, materials, rootObject, dir);

            spdlog::info("Successfully loaded OBJ model: {}", pPath);

        } else {
            spdlog::error("Unsupported model format: {}", pPath);
            rootObject->Destroy();
            return nullptr;
        }

        return rootObject;
    }


}; // namespace golias
