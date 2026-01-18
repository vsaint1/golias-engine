
#include "core/model.h"

#include "core/engine.h"
#include "core/graphics/material.h"
#include "core/graphics/structs.h"
#include "scene/3d/animation_component.h"
#include "scene/3d/mesh_component.h"
#include "scene/3d/skeleton_animation_component.h"
#include "scene/game_object.h"
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <unordered_set>

namespace golias {


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

    // ============================================================================
    // GLTF Data Extraction
    // ============================================================================

    bool ExtractScalarData(const cgltf_accessor* accessor, std::vector<float>& out) {
        if (!accessor || !accessor->buffer_view) {
            return false;
        }

        out.resize(accessor->count);
        for (size_t i = 0; i < accessor->count; ++i) {
            cgltf_accessor_read_float(accessor, i, &out[i], 1);
        }

        return true;
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

    bool ExtractVec3Data(const cgltf_accessor* accessor, std::vector<glm::vec3>& out) {
        if (!accessor || !accessor->buffer_view) {
            return false;
        }

        out.resize(accessor->count);
        for (size_t i = 0; i < accessor->count; ++i) {
            float vec[3] = {0.0f, 0.0f, 0.0f};
            cgltf_accessor_read_float(accessor, i, vec, 3);
            out[i] = glm::vec3(vec[0], vec[1], vec[2]);
        }

        return true;
    }

    bool ExtractQuatData(const cgltf_accessor* accessor, std::vector<glm::quat>& out) {
        if (!accessor || !accessor->buffer_view) {
            return false;
        }

        out.resize(accessor->count);
        for (size_t i = 0; i < accessor->count; ++i) {
            float vec[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            cgltf_accessor_read_float(accessor, i, vec, 4);
            out[i] = glm::quat(vec[3], vec[0], vec[1], vec[2]);
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

    // ============================================================================
    // Vertex Layout and Data Structures
    // ============================================================================

    VertexLayout CreateStandardVertexLayout() {
        VertexLayout layout;
        Uint32 offset = 0;

        layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
        offset += 8;

        layout.elements.push_back({3, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        layout.stride = offset;
        return layout;
    }

    VertexLayout CreateSkinnedVertexLayout() {
        VertexLayout layout;
        Uint32 offset = 0;

        // Position
        layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        // Color
        layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        // TexCoord
        layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
        offset += 8;

        // Normal
        layout.elements.push_back({3, 3, EDataType::FLOAT, false, offset});
        offset += 12;

        // Bone Indices (4 ints)
        layout.elements.push_back({4, 4, EDataType::FLOAT, false, offset});
        offset += 16;

        // Bone Weights (4 floats)
        layout.elements.push_back({5, 4, EDataType::FLOAT, false, offset});
        offset += 16;

        layout.stride = offset;
        return layout;
    }

    struct VertexAttributeData {
        std::vector<float> positions;
        std::vector<float> colors;
        std::vector<float> texcoords;
        std::vector<float> normals;
        std::vector<float> boneIndices;
        std::vector<float> boneWeights;
        size_t vertexCount = 0;
        bool hasSkinning   = false;

        const cgltf_accessor* positionAccessor = nullptr;
        const cgltf_accessor* colorAccessor    = nullptr;
        const cgltf_accessor* texcoordAccessor = nullptr;
        const cgltf_accessor* normalAccessor   = nullptr;
        const cgltf_accessor* jointsAccessor   = nullptr;
        const cgltf_accessor* weightsAccessor  = nullptr;
    };

    bool ExtractGLTFVertexData(const cgltf_primitive* prim, VertexAttributeData& data) {
        for (size_t i = 0; i < prim->attributes_count; ++i) {
            const cgltf_attribute& a = prim->attributes[i];
            if (!a.data) {
                continue;
            }

            switch (a.type) {
            case cgltf_attribute_type_position:
                data.positionAccessor = a.data;
                data.vertexCount      = a.data->count;
                break;
            case cgltf_attribute_type_normal:
                data.normalAccessor = a.data;
                break;
            case cgltf_attribute_type_texcoord:
                if (!data.texcoordAccessor) {
                    data.texcoordAccessor = a.data;
                }
                break;
            case cgltf_attribute_type_color:
                if (!data.colorAccessor) {
                    data.colorAccessor = a.data;
                }
                break;
            case cgltf_attribute_type_joints:
                if (!data.jointsAccessor) {
                    data.jointsAccessor = a.data;
                    data.hasSkinning    = true;
                }
                break;
            case cgltf_attribute_type_weights:
                if (!data.weightsAccessor) {
                    data.weightsAccessor = a.data;
                    data.hasSkinning     = true;
                }
                break;
            default:
                break;
            }
        }

        if (data.vertexCount == 0) {
            return false;
        }

        if (data.positionAccessor) {
            ExtractFloatData(data.positionAccessor, data.positions);
        }
        if (data.normalAccessor) {
            ExtractFloatData(data.normalAccessor, data.normals);
        }
        if (data.texcoordAccessor) {
            ExtractFloatData(data.texcoordAccessor, data.texcoords);
        }
        if (data.colorAccessor) {
            ExtractFloatData(data.colorAccessor, data.colors);
        }
        if (data.jointsAccessor) {
            ExtractFloatData(data.jointsAccessor, data.boneIndices);
        }
        if (data.weightsAccessor) {
            ExtractFloatData(data.weightsAccessor, data.boneWeights);
        }

        return true;
    }

    std::vector<float> InterleaveVertexData(const VertexAttributeData& data) {
        std::vector<float> vertices;
        size_t floatsPerVertex = data.hasSkinning ? 19 : 11; // 3+3+2+3 = 11, +4+4 = 19 for skinned
        vertices.reserve(data.vertexCount * floatsPerVertex);

        for (size_t v = 0; v < data.vertexCount; ++v) {
            // Position (3 floats)
            if (v * 3 + 2 < data.positions.size()) {
                vertices.push_back(data.positions[v * 3]);
                vertices.push_back(data.positions[v * 3 + 1]);
                vertices.push_back(data.positions[v * 3 + 2]);
            } else {
                vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});
            }

            // Color (3 floats)
            if (!data.colors.empty() && data.colorAccessor) {
                size_t colorComps = GetComponentCount(data.colorAccessor->type);
                if (colorComps == 3 && v * 3 + 2 < data.colors.size()) {
                    vertices.push_back(data.colors[v * 3]);
                    vertices.push_back(data.colors[v * 3 + 1]);
                    vertices.push_back(data.colors[v * 3 + 2]);
                } else if (colorComps == 4 && v * 4 + 2 < data.colors.size()) {
                    vertices.push_back(data.colors[v * 4]);
                    vertices.push_back(data.colors[v * 4 + 1]);
                    vertices.push_back(data.colors[v * 4 + 2]);
                } else {
                    vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});
                }
            } else {
                vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});
            }

            // Texcoord (2 floats)
            if (!data.texcoords.empty() && data.texcoordAccessor) {
                size_t texcoordComps = GetComponentCount(data.texcoordAccessor->type);
                if (texcoordComps == 2 && v * 2 + 1 < data.texcoords.size()) {
                    vertices.push_back(data.texcoords[v * 2]);
                    vertices.push_back(data.texcoords[v * 2 + 1]);
                } else if (texcoordComps == 3 && v * 3 + 1 < data.texcoords.size()) {
                    vertices.push_back(data.texcoords[v * 3]);
                    vertices.push_back(data.texcoords[v * 3 + 1]);
                } else {
                    vertices.insert(vertices.end(), {0.0f, 0.0f});
                }
            } else {
                vertices.insert(vertices.end(), {0.0f, 0.0f});
            }

            // Normal (3 floats)
            if (!data.normals.empty() && data.normalAccessor && v * 3 + 2 < data.normals.size()) {
                vertices.push_back(data.normals[v * 3]);
                vertices.push_back(data.normals[v * 3 + 1]);
                vertices.push_back(data.normals[v * 3 + 2]);
            } else {
                vertices.insert(vertices.end(), {0.0f, 1.0f, 0.0f});
            }

            // Bone Indices (4 floats - will be interpreted as ints in shader)
            if (data.hasSkinning && !data.boneIndices.empty()) {
                size_t jointComps = GetComponentCount(data.jointsAccessor->type);
                if (jointComps == 4 && v * 4 + 3 < data.boneIndices.size()) {
                    vertices.push_back(data.boneIndices[v * 4]);
                    vertices.push_back(data.boneIndices[v * 4 + 1]);
                    vertices.push_back(data.boneIndices[v * 4 + 2]);
                    vertices.push_back(data.boneIndices[v * 4 + 3]);
                } else {
                    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f});
                }
            } else if (data.hasSkinning) {
                vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f});
            }

            // Bone Weights (4 floats)
            if (data.hasSkinning && !data.boneWeights.empty()) {
                size_t weightComps = GetComponentCount(data.weightsAccessor->type);
                if (weightComps == 4 && v * 4 + 3 < data.boneWeights.size()) {
                    vertices.push_back(data.boneWeights[v * 4]);
                    vertices.push_back(data.boneWeights[v * 4 + 1]);
                    vertices.push_back(data.boneWeights[v * 4 + 2]);
                    vertices.push_back(data.boneWeights[v * 4 + 3]);
                } else {
                    vertices.insert(vertices.end(), {1.0f, 0.0f, 0.0f, 0.0f});
                }
            } else if (data.hasSkinning) {
                vertices.insert(vertices.end(), {1.0f, 0.0f, 0.0f, 0.0f});
            }
        }

        return vertices;
    }

    // ============================================================================
    // Base64 Decoding
    // ============================================================================

    std::vector<uint8_t> Base64Decode(const std::string& encoded) {
        static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                "abcdefghijklmnopqrstuvwxyz"
                                                "0123456789+/";

        std::vector<uint8_t> decoded;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) {
            T[base64_chars[i]] = i;
        }

        int val = 0, valb = -8;
        for (unsigned char c : encoded) {
            if (T[c] == -1) {
                break;
            }
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                decoded.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return decoded;
    }

    // ============================================================================
    // Texture Loading
    // ============================================================================

    std::shared_ptr<Texture2D>
        LoadGLTFTexture(cgltf_texture* texture, const std::string& base_path, const std::string& fallback_name = "") {

        if (!texture || !texture->image) {
            return nullptr;
        }

        auto& texture_manager = Engine::GetInstance().GetTextureManager();
        cgltf_image* image    = texture->image;

        // Handle data URI
        if (image->uri) {
            std::string uri_str(image->uri);

            if (uri_str.find("data:") == 0) {
                spdlog::info("Found data URI texture: {}", fallback_name.empty() ? "unnamed" : fallback_name);

                size_t comma_pos = uri_str.find(',');
                if (comma_pos == std::string::npos) {
                    spdlog::warn("Invalid data URI format for {}", fallback_name);
                    return nullptr;
                }

                std::string base64_data      = uri_str.substr(comma_pos + 1);
                std::vector<uint8_t> decoded = Base64Decode(base64_data);

                if (decoded.empty()) {
                    spdlog::warn("Failed to decode base64 data for {}", fallback_name);
                    return nullptr;
                }

                int width, height, channels;
                stbi_uc* image_data =
                    stbi_load_from_memory(decoded.data(), static_cast<int>(decoded.size()), &width, &height, &channels, 0);

                if (!image_data) {
                    spdlog::warn("Failed to decode data URI texture {}: {}", fallback_name, stbi_failure_reason());
                    return nullptr;
                }

                std::string embedded_path = "datauri://" + (fallback_name.empty() ? "unnamed_texture" : fallback_name);

                ETextureFormat format = TextureFormatFromChannels(channels);

                auto loaded_texture = texture_manager.EnsureTexture2D(embedded_path, width, height, format, image_data);

                if (loaded_texture) {
                    spdlog::debug("Successfully created data URI texture: {} ({}x{} {} channels)", embedded_path, width, height, channels);
                    return loaded_texture;
                } else {
                    SDL_free(image_data);
                    spdlog::warn("Failed to create texture from data URI {}: unknown_error", fallback_name);
                }

                return nullptr;
            } else {
                // External file
                std::string tex_path = base_path + image->uri;
                auto loaded_texture  = texture_manager.EnsureTexture2D(tex_path);
                if (loaded_texture) {
                    spdlog::debug("Loaded external texture: {}", tex_path);
                    return loaded_texture;
                } else {
                    spdlog::warn("Failed to load external texture {}: unknown_error", tex_path);
                }

                return nullptr;
            }
        }

        // Handle embedded buffer view
        if (image->buffer_view) {
            spdlog::info("Loading embedded texture from buffer view: {} ({} bytes)",
                         fallback_name.empty() ? "unnamed" : fallback_name,
                         image->buffer_view->size);

            const uint8_t* data = static_cast<const uint8_t*>(image->buffer_view->buffer->data) + image->buffer_view->offset;
            size_t size         = image->buffer_view->size;

            int width, height, channels;

            stbi_uc* image_data = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 0);

            if (!image_data) {
                spdlog::warn("Failed to decode embedded texture {}: {}", fallback_name, stbi_failure_reason());
                return nullptr;
            }

            std::string embedded_path = "embedded://";

            if (!base_path.empty()) {
                size_t last_slash = base_path.find_last_of("/\\");
                if (last_slash != std::string::npos && last_slash + 1 < base_path.length()) {
                    std::string model_name = base_path.substr(last_slash + 1);
                    size_t dot_pos         = model_name.find_last_of('.');
                    if (dot_pos != std::string::npos) {
                        model_name = model_name.substr(0, dot_pos);
                    }
                    embedded_path += model_name + "/";
                }
            }

            embedded_path += fallback_name.empty() ? "unnamed_texture" : fallback_name;
            ETextureFormat format = TextureFormatFromChannels(channels);
            auto loaded_texture   = texture_manager.EnsureTexture2D(embedded_path, width, height, format, image_data);

            if (loaded_texture) {
                spdlog::debug("Successfully created embedded texture: {} ({}x{} {} channels)", embedded_path, width, height, channels);
            } else {
                SDL_free(image_data);
                spdlog::warn("Failed to create texture from embedded data {}: unknown error", fallback_name);
            }

            return loaded_texture;
        }

        spdlog::warn("Texture has neither URI nor buffer_view: {}", fallback_name);
        return nullptr;
    }

    // ============================================================================
    // Mesh Component Creation
    // ============================================================================
    void CreateMeshComponentGLTF(GameObject* gameObject,
                                 const VertexLayout& layout,
                                 const std::vector<float>& vertices,
                                 const std::vector<Uint32>& indices,
                                 cgltf_material* gltf_material,
                                 const std::string& base_path) {

        auto& engine = Engine::GetInstance();
        auto rd      = engine.GetSceneRenderer().GetRenderingDevice();

        std::shared_ptr<Mesh> mesh         = rd->CreateMeshFromData(layout, vertices, indices);
        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
        material->SetShader(shader);

        glm::vec4 base_color(1.0f);
        float metallic  = 0.0f;
        float roughness = 1.0f;
        glm::vec3 emissive(0.0f);
        float emissiveStrength     = 1.0f;
        ETextureFlags textureFlags = ETextureFlags::NONE;

        // Default material state
        EBlendMode blendMode     = EBlendMode::BLEND_MODE_OPAQUE;
        bool depthWrite          = true;
        ECullMode cullMode       = ECullMode::CULL_MODE_BACK;
        float alphaClipThreshold = 0.5f;

        if (gltf_material) {
            if (gltf_material->alpha_mode == cgltf_alpha_mode_blend) {
                blendMode  = EBlendMode::BLEND_MODE_ALPHA;
                depthWrite = false;
                spdlog::debug("Material '{}' uses BLEND mode", gltf_material->name ? gltf_material->name : "unnamed");
            } else if (gltf_material->alpha_mode == cgltf_alpha_mode_mask) {
                blendMode          = EBlendMode::BLEND_MODE_OPAQUE;
                depthWrite         = true;
                alphaClipThreshold = gltf_material->alpha_cutoff;
                spdlog::debug("Material '{}' uses MASK mode with cutoff: {}",
                              gltf_material->name ? gltf_material->name : "unnamed",
                              alphaClipThreshold);
            } else {
                blendMode  = EBlendMode::BLEND_MODE_OPAQUE;
                depthWrite = true;
            }

            if (gltf_material->double_sided) {
                cullMode = ECullMode::CULL_MODE_DISABLED;
                spdlog::debug("Material '{}' is double-sided", gltf_material->name ? gltf_material->name : "unnamed");
            }

            // Parse PBR properties
            if (gltf_material->has_pbr_metallic_roughness) {
                auto& pbr = gltf_material->pbr_metallic_roughness;
                base_color =
                    glm::vec4(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3]);
                metallic  = pbr.metallic_factor;
                roughness = pbr.roughness_factor;

                if (base_color.a < 1.0f && blendMode == EBlendMode::BLEND_MODE_OPAQUE) {
                    spdlog::warn("Material '{}' has alpha < 1.0 but alpha_mode is OPAQUE. "
                                 "Consider using BLEND mode.",
                                 gltf_material->name ? gltf_material->name : "unnamed");
                }

                // Albedo texture
                if (pbr.base_color_texture.texture) {
                    std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_albedo" : "albedo";
                    auto texture         = LoadGLTFTexture(pbr.base_color_texture.texture, base_path, tex_name);
                    if (texture) {
                        material->SetParameter("ALBEDO_TEXTURE", texture);
                        textureFlags |= ETextureFlags::HAS_ALBEDO;
                    }
                }

                // Metallic-Roughness texture
                if (pbr.metallic_roughness_texture.texture) {
                    std::string tex_name =
                        gltf_material->name ? std::string(gltf_material->name) + "_metallic_roughness" : "metallic_roughness";
                    auto texture = LoadGLTFTexture(pbr.metallic_roughness_texture.texture, base_path, tex_name);
                    if (texture) {
                        material->SetParameter("METALLIC_TEXTURE", texture);
                        material->SetParameter("ROUGHNESS_TEXTURE", texture);
                        textureFlags |= ETextureFlags::HAS_METALLIC;
                        textureFlags |= ETextureFlags::HAS_ROUGHNESS;
                    }
                }
            }

            // Normal map
            if (gltf_material->normal_texture.texture) {
                std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_normal" : "normal";
                auto texture         = LoadGLTFTexture(gltf_material->normal_texture.texture, base_path, tex_name);
                if (texture) {
                    material->SetParameter("NORMAL_TEXTURE", texture);
                    textureFlags |= ETextureFlags::HAS_NORMAL;
                }
            }

            // Occlusion map
            if (gltf_material->occlusion_texture.texture) {
                std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_occlusion" : "occlusion";
                auto texture         = LoadGLTFTexture(gltf_material->occlusion_texture.texture, base_path, tex_name);
                if (texture) {
                    material->SetParameter("AO_TEXTURE", texture);
                    textureFlags |= ETextureFlags::HAS_AO;
                }
            }

            // Emissive
            emissive = glm::vec3(gltf_material->emissive_factor[0], gltf_material->emissive_factor[1], gltf_material->emissive_factor[2]);

            if (gltf_material->has_emissive_strength) {
                emissiveStrength = gltf_material->emissive_strength.emissive_strength;
            }

            if (gltf_material->emissive_texture.texture) {
                std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_emissive" : "emissive";
                auto texture         = LoadGLTFTexture(gltf_material->emissive_texture.texture, base_path, tex_name);
                if (texture) {
                    material->SetParameter("EMISSIVE_TEXTURE", texture);
                    textureFlags |= ETextureFlags::HAS_EMISSIVE;
                }
            }
        }

        bool shouldUseIBL = false;

        if (gltf_material) {
            if (gltf_material->unlit) {
                shouldUseIBL = false;
                spdlog::debug("Material '{}' is unlit, disabling IBL", gltf_material->name ? gltf_material->name : "unnamed");
            } else {

                // Only enable IBL if material has STRONG indicators of needing it
                // 1. High metallic materials NEED IBL (mirrors, chrome, etc.)
                if (metallic > 0.5f) {
                    shouldUseIBL = true;
                    spdlog::debug("Material '{}' has high metallic ({:.2f}), enabling IBL",
                                  gltf_material->name ? gltf_material->name : "unnamed",
                                  metallic);
                } // 2. Very smooth materials benefit from IBL (polished surfaces)
                else if (roughness < 0.3f) {
                    shouldUseIBL = true;
                    spdlog::debug("Material '{}' has low roughness ({:.2f}), enabling IBL",
                                  gltf_material->name ? gltf_material->name : "unnamed",
                                  roughness);
                } // 3. If material has metallic/roughness texture, it's probably PBR
                else if (gltf_material->has_pbr_metallic_roughness
                         && gltf_material->pbr_metallic_roughness.metallic_roughness_texture.texture) {
                    shouldUseIBL = true;
                    spdlog::debug("Material '{}' has metallic/roughness texture, enabling IBL",
                                  gltf_material->name ? gltf_material->name : "unnamed");
                } // Otherwise, default to studio lighting (no IBL)
                else {
                    shouldUseIBL = false;
                    spdlog::debug("Material '{}' using studio lighting (metallic: {:.2f}, "
                                  "roughness: {:.2f})",
                                  gltf_material->name ? gltf_material->name : "unnamed",
                                  metallic,
                                  roughness);
                }
            }
        } else {
            // No material data - default to no IBL for studio lighting
            shouldUseIBL = false;
            spdlog::debug("No material data, using studio lighting");
        }

        material->SetImageBasedLighting(shouldUseIBL);

        material->SetParameter("TEXTURE_FLAGS", static_cast<int>(textureFlags));
        material->SetParameter("u_material.modulate", base_color);
        material->SetParameter("u_material.metallicFactor", metallic);
        material->SetParameter("u_material.roughnessFactor", roughness);
        material->SetParameter("u_material.emissiveFactor", emissive);
        material->SetParameter("u_material.emissiveStrength", emissiveStrength);

        material->SetBlendMode(blendMode);
        material->SetDepthWriteEnabled(depthWrite);
        material->SetCullMode(cullMode);
        material->SetAlphaClipThreshold(alphaClipThreshold);
        material->SetDepthTestEnabled(true);
        material->SetDepthFunc(EComparisonFunc::COMPARISON_LESS);

        gameObject->AddComponent(new MeshComponent(mesh, material));
    }

    void CreateMeshComponentOBJ(GameObject* gameObject,
                                const VertexLayout& layout,
                                const std::vector<float>& vertices,
                                const std::vector<Uint32>& indices,
                                const tinyobj::material_t* obj_material,
                                const std::string& base_path) {
                                    
        auto& engine = Engine::GetInstance();
        auto rd      = engine.GetSceneRenderer().GetRenderingDevice();

        std::shared_ptr<Mesh> mesh         = rd->CreateMeshFromData(layout, vertices, indices);
        std::shared_ptr<Material> material = std::make_shared<Material>();
        std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
        material->SetShader(shader);

        ETextureFlags textureFlags = ETextureFlags::NONE;
        glm::vec4 base_color(1.0f);
        float roughness = 1.0f;
        float shininess = 0.0f;

        // Default material state
        EBlendMode blendMode = EBlendMode::BLEND_MODE_OPAQUE;
        bool depthWrite      = true;
        ECullMode cullMode   = ECullMode::CULL_MODE_BACK;

        if (obj_material) {
            float alpha = obj_material->dissolve;
            base_color  = glm::vec4(obj_material->diffuse[0], obj_material->diffuse[1], obj_material->diffuse[2], alpha);

            if (alpha < 1.0f) {
                blendMode  = EBlendMode::BLEND_MODE_ALPHA;
                depthWrite = false;
                spdlog::debug("OBJ material has transparency: alpha = {}", alpha);
            }

            if (!obj_material->diffuse_texname.empty()) {
                std::string tex_path = base_path + obj_material->diffuse_texname;
                auto texture         = engine.GetTextureManager().EnsureTexture2D(tex_path);
                if (texture) {
                    material->SetParameter("ALBEDO_TEXTURE", texture);
                    textureFlags |= ETextureFlags::HAS_ALBEDO;
                }
            }

            if (!obj_material->normal_texname.empty()) {
                std::string tex_path = base_path + obj_material->normal_texname;
                auto texture         = engine.GetTextureManager().EnsureTexture2D(tex_path);
                if (texture) {
                    material->SetParameter("NORMAL_TEXTURE", texture);
                    textureFlags |= ETextureFlags::HAS_NORMAL;
                }
            }

            if (!obj_material->alpha_texname.empty()) {
                std::string tex_path = base_path + obj_material->alpha_texname;
                auto texture         = engine.GetTextureManager().EnsureTexture2D(tex_path);
                if (texture) {
                    spdlog::info("Found alpha texture in OBJ: {}", obj_material->alpha_texname);
                    blendMode  = EBlendMode::BLEND_MODE_ALPHA;
                    depthWrite = false;
                }
            }

            shininess = obj_material->shininess;
            roughness = 1.0f - glm::clamp(shininess / 1000.0f, 0.0f, 1.0f);
        }

        bool shouldUseIBL = false;

        if (obj_material) {
            // OBJ doesn't have metallic values, so we use shininess as a heuristic
            // Very shiny materials (chrome, mirrors) should use IBL
            if (shininess > 500.0f) {
                shouldUseIBL = true;
                spdlog::debug("OBJ material has high shininess ({:.1f}), enabling IBL", shininess);
            } // Materials with specular highlights benefit from IBL
            else if (obj_material->specular[0] > 0.5f || obj_material->specular[1] > 0.5f || obj_material->specular[2] > 0.5f) {
                shouldUseIBL = true;
                spdlog::debug("OBJ material has strong specular, enabling IBL");
            } // Otherwise use studio lighting
            else {
                shouldUseIBL = false;
                spdlog::debug("OBJ material using studio lighting (shininess: {:.1f})", shininess);
            }
        } else {
            // No material - use studio lighting
            shouldUseIBL = false;
            spdlog::debug("OBJ has no material, using studio lighting");
        }

        material->SetImageBasedLighting(shouldUseIBL);

        material->SetParameter("TEXTURE_FLAGS", static_cast<int>(textureFlags));
        material->SetParameter("u_material.modulate", base_color);
        material->SetParameter("u_material.metallicFactor", 0.0f);
        material->SetParameter("u_material.roughnessFactor", roughness);
        material->SetParameter("u_material.emissiveFactor", glm::vec3(0.0f));
        material->SetParameter("u_material.emissiveStrength", 1.0f);

        material->SetBlendMode(blendMode);
        material->SetDepthWriteEnabled(depthWrite);
        material->SetCullMode(cullMode);
        material->SetDepthTestEnabled(true);
        material->SetDepthFunc(EComparisonFunc::COMPARISON_LESS);
        material->SetAlphaClipThreshold(0.5f);

        gameObject->AddComponent(new MeshComponent(mesh, material));
    }

    // ============================================================================
    // GLTF Animation Loading
    // ============================================================================
    void LoadGLTFAnimations(cgltf_data* data, GameObject* rootObject) {
        std::vector<std::shared_ptr<AnimationClip>> clips;

        for (cgltf_size ai = 0; ai < data->animations_count; ++ai) {
            auto& anim     = data->animations[ai];
            auto clip      = std::make_shared<AnimationClip>();
            clip->name     = anim.name ? anim.name : "no_name";
            clip->duration = 0.0f;

            spdlog::debug("Processing GLTF Animation clip: {} with {} channels", clip->name, anim.channels_count);

            std::unordered_map<cgltf_node*, size_t> trackIndexOf;

            auto ensureTrack = [&](cgltf_node* node) -> TransformTrack& {
                auto it = trackIndexOf.find(node);
                if (it != trackIndexOf.end()) {
                    return clip->tracks[it->second];
                }

                TransformTrack track;
                track.targetName = node->name ? node->name : "";
                clip->tracks.push_back(track);
                size_t idx         = clip->tracks.size() - 1;
                trackIndexOf[node] = idx;
                return clip->tracks[idx];
            };

            for (cgltf_size ci = 0; ci < anim.channels_count; ++ci) {
                auto& channel = anim.channels[ci];
                auto sampler  = channel.sampler;

                if (!channel.target_node || !sampler || !sampler->input || !sampler->output) {
                    continue;
                }

                std::vector<float> times;
                if (!ExtractScalarData(sampler->input, times)) {
                    spdlog::warn("Failed to extract animation times for clip: {} channel: {}", clip->name, ci);
                    continue;
                }

                auto& track = ensureTrack(channel.target_node);

                switch (channel.target_path) {
                case cgltf_animation_path_type_translation:
                    {
                        std::vector<glm::vec3> values;
                        ExtractVec3Data(sampler->output, values);

                        if (!values.empty()) {
                            track.positions.resize(times.size());
                            for (size_t i = 0; i < times.size(); ++i) {
                                track.positions[i].time  = times[i];
                                track.positions[i].value = values[i];
                            }
                        }
                    }
                    break;

                case cgltf_animation_path_type_rotation:
                    {
                        std::vector<glm::quat> values;
                        ExtractQuatData(sampler->output, values);

                        if (!values.empty()) {
                            track.rotations.resize(times.size());
                            for (size_t i = 0; i < times.size(); ++i) {
                                track.rotations[i].time  = times[i];
                                track.rotations[i].value = values[i];
                            }
                        }
                    }
                    break;

                case cgltf_animation_path_type_scale:
                    {
                        std::vector<glm::vec3> values;
                        ExtractVec3Data(sampler->output, values);

                        if (!values.empty()) {
                            track.scales.resize(times.size());
                            for (size_t i = 0; i < times.size(); ++i) {
                                track.scales[i].time  = times[i];
                                track.scales[i].value = values[i];
                            }
                        }
                    }
                    break;

                default:
                    break;
                }

                clip->duration = SDL_max(clip->duration, times.back());
            }

            clips.push_back(std::move(clip));
        }

        if (!clips.empty()) {
            auto animComp = new AnimationComponent();
            rootObject->AddComponent(animComp);

            for (auto& clip : clips) {
                animComp->RegisterClip(clip->name, clip);
            }

            spdlog::debug("Loaded {} Animation clips from GLTF file", data->animations_count);
        }
    }

    // ============================================================================
    // GLTF Skeleton Loading
    // ============================================================================
    void LoadGLTFSkeleton(cgltf_data* data, GameObject* rootObject) {
        // Check if the model has any skins (skeletal data)
        if (data->skins_count == 0) {
            return;
        }

        std::vector<std::shared_ptr<Skeleton>> skeletons;
        std::vector<std::shared_ptr<SkeletonAnimationClip>> skeletonClips;

        std::unordered_set<cgltf_node*> jointNodes;

        for (cgltf_size si = 0; si < data->skins_count; ++si) {
            auto& skin     = data->skins[si];
            auto skeleton  = std::make_shared<Skeleton>();
            skeleton->name = skin.name ? skin.name : "Skeleton";

            spdlog::info("Processing GLTF Skin/Skeleton: {} with {} joints", skeleton->name, skin.joints_count);

            // Build joint hierarchy
            std::unordered_map<cgltf_node*, int> nodeToJointIndex;

            for (cgltf_size ji = 0; ji < skin.joints_count; ++ji) {
                cgltf_node* jointNode       = skin.joints[ji];
                nodeToJointIndex[jointNode] = static_cast<int>(ji);
                jointNodes.insert(jointNode); // Track all joint nodes

                SkeletonJoint joint;
                joint.name = jointNode->name ? jointNode->name : ("Joint_" + std::to_string(ji));

                if (jointNode->has_translation) {
                    joint.position = glm::vec3(jointNode->translation[0], jointNode->translation[1], jointNode->translation[2]);
                }

                if (jointNode->has_rotation) {
                    joint.rotation =
                        glm::quat(jointNode->rotation[3], jointNode->rotation[0], jointNode->rotation[1], jointNode->rotation[2]);
                }

                if (jointNode->has_scale) {
                    joint.scale = glm::vec3(jointNode->scale[0], jointNode->scale[1], jointNode->scale[2]);
                }

                if (skin.inverse_bind_matrices) {
                    float mat[16];
                    cgltf_accessor_read_float(skin.inverse_bind_matrices, ji, mat, 16);
                    joint.inverseBindMatrix = glm::make_mat4(mat);
                }

                skeleton->joints.push_back(joint);
            }

            for (cgltf_size ji = 0; ji < skin.joints_count; ++ji) {
                cgltf_node* jointNode = skin.joints[ji];
                if (jointNode->parent) {
                    auto parentIt = nodeToJointIndex.find(jointNode->parent);
                    if (parentIt != nodeToJointIndex.end()) {
                        skeleton->joints[ji].parentIndex = parentIt->second;
                    }
                }
            }

            skeletons.push_back(skeleton);
        }

        for (cgltf_size ai = 0; ai < data->animations_count; ++ai) {
            auto& anim     = data->animations[ai];
            auto clip      = std::make_shared<SkeletonAnimationClip>();
            clip->name     = anim.name ? anim.name : "SkeletonAnimation";
            clip->duration = 0.0f;

            std::unordered_map<cgltf_node*, size_t> trackIndexOf;

            auto ensureTrack = [&](cgltf_node* node) -> SkeletonAnimationTrack& {
                auto it = trackIndexOf.find(node);
                if (it != trackIndexOf.end()) {
                    return clip->tracks[it->second];
                }

                SkeletonAnimationTrack track;
                track.targetJointName = node->name ? node->name : "";
                clip->tracks.push_back(track);
                size_t idx         = clip->tracks.size() - 1;
                trackIndexOf[node] = idx;
                return clip->tracks[idx];
            };

            bool hasSkeletonAnimation = false;

            for (cgltf_size ci = 0; ci < anim.channels_count; ++ci) {
                auto& channel = anim.channels[ci];
                auto sampler  = channel.sampler;

                if (!channel.target_node || !sampler || !sampler->input || !sampler->output) {
                    continue;
                }

                bool isJoint = jointNodes.find(channel.target_node) != jointNodes.end();

                if (!isJoint) {
                    continue;
                }

                hasSkeletonAnimation = true;

                std::vector<float> times;
                if (!ExtractScalarData(sampler->input, times)) {
                    spdlog::warn("Failed to extract skeleton animation times for clip: {} "
                                 "channel: {}",
                                 clip->name,
                                 ci);
                    continue;
                }

                auto& track = ensureTrack(channel.target_node);

                switch (channel.target_path) {
                case cgltf_animation_path_type_translation:
                    {
                        std::vector<glm::vec3> values;
                        ExtractVec3Data(sampler->output, values);

                        if (!values.empty()) {
                            track.positions.resize(times.size());
                            for (size_t i = 0; i < times.size(); ++i) {
                                track.positions[i].time  = times[i];
                                track.positions[i].value = values[i];
                            }
                        }
                    }
                    break;

                case cgltf_animation_path_type_rotation:
                    {
                        std::vector<glm::quat> values;
                        ExtractQuatData(sampler->output, values);

                        if (!values.empty()) {
                            track.rotations.resize(times.size());
                            for (size_t i = 0; i < times.size(); ++i) {
                                track.rotations[i].time  = times[i];
                                track.rotations[i].value = values[i];
                            }
                        }
                    }
                    break;

                case cgltf_animation_path_type_scale:
                    {
                        std::vector<glm::vec3> values;
                        ExtractVec3Data(sampler->output, values);

                        if (!values.empty()) {
                            track.scales.resize(times.size());
                            for (size_t i = 0; i < times.size(); ++i) {
                                track.scales[i].time  = times[i];
                                track.scales[i].value = values[i];
                            }
                        }
                    }
                    break;

                default:
                    break;
                }

                clip->duration = SDL_max(clip->duration, times.back());
            }

            if (hasSkeletonAnimation) {
                skeletonClips.push_back(std::move(clip));
            }
        }

        if (!skeletons.empty() && !skeletonClips.empty()) {
            auto skelAnimComp = new SkeletonAnimationComponent();
            rootObject->AddComponent(skelAnimComp);

            skelAnimComp->SetSkeleton(skeletons[0]);

            for (auto& clip : skeletonClips) {
                skelAnimComp->RegisterClip(clip->name, clip);
                spdlog::debug(
                    "Registered skeleton animation clip: {} with {} tracks, duration: {}", clip->name, clip->tracks.size(), clip->duration);
            }

            spdlog::debug("Loaded {} Skeleton animation clips from GLTF file", skeletonClips.size());
        } else if (!skeletons.empty()) {
            auto skelAnimComp = new SkeletonAnimationComponent();
            rootObject->AddComponent(skelAnimComp);
            skelAnimComp->SetSkeleton(skeletons[0]);
            spdlog::debug("Loaded Skeleton with {} joints (no animations)", skeletons[0]->joints.size());
        }

        spdlog::debug("Processed {} skins/skeletons from GLTF file", data->skins_count);
    }

    // ============================================================================
    // GLTF Scene Graph Parsing
    // ============================================================================
    void ParseGLTFNode(cgltf_node* node, GameObject* parent, cgltf_data* data, const std::string& base_path, Scene* scene) {
        std::string node_name  = node->name ? node->name : "Node";
        GameObject* nodeObject = scene->CreateObject(node_name, parent);

        glm::vec3 pos(0.0f);
        glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scl(1.0f);

        if (node->has_translation) {
            pos = glm::vec3(node->translation[0], node->translation[1], node->translation[2]);
        }
        if (node->has_rotation) {
            rot = glm::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]);
        }
        if (node->has_scale) {
            scl = glm::vec3(node->scale[0], node->scale[1], node->scale[2]);
        }

        if (node->has_matrix && !node->has_translation && !node->has_rotation && !node->has_scale) {
            glm::mat4 mat = glm::make_mat4(node->matrix);
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(mat, scl, rot, pos, skew, perspective);
        }

        nodeObject->SetPosition(pos);
        nodeObject->SetRotation(rot);
        nodeObject->SetScale(scl);

        if (node->mesh) {
            cgltf_mesh* mesh_data = node->mesh;

            for (cgltf_size p = 0; p < mesh_data->primitives_count; ++p) {
                cgltf_primitive* prim = &mesh_data->primitives[p];

                VertexAttributeData vertexData;
                if (!ExtractGLTFVertexData(prim, vertexData)) {
                    continue;
                }

                std::vector<float> vertices = InterleaveVertexData(vertexData);
                VertexLayout layout         = vertexData.hasSkinning ? CreateSkinnedVertexLayout() : CreateStandardVertexLayout();

                std::vector<Uint32> indices;
                EDataType index_type;
                if (prim->indices) {
                    ExtractIndexData(prim->indices, indices, index_type);
                }

                CreateMeshComponentGLTF(nodeObject, layout, vertices, indices, prim->material, base_path);
            }
        }

        for (cgltf_size c = 0; c < node->children_count; ++c) {
            ParseGLTFNode(node->children[c], nodeObject, data, base_path, scene);
        }
    }

    // ============================================================================
    // OBJ Parsing
    // ============================================================================
    void ParseOBJ(const tinyobj::attrib_t& attrib,
                  const std::vector<tinyobj::shape_t>& staticMeshes,
                  const std::vector<tinyobj::material_t>& materials,
                  GameObject* rootObject,
                  const std::string& base_path,
                  Scene* scene) {

        for (const auto& staticMesh : staticMeshes) {
            GameObject* staticMeshObject = scene->CreateObject(staticMesh.name.empty() ? "SM_" : staticMesh.name, rootObject);
            const auto& mesh             = staticMesh.mesh;

            VertexLayout layout = CreateStandardVertexLayout();
            std::vector<float> vertices;
            std::vector<Uint32> indices;
            size_t vertex_offset = 0;

            size_t index_offset = 0;
            for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
                size_t fv = mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; ++v) {
                    tinyobj::index_t idx = mesh.indices[index_offset + v];

                    // Position
                    if (idx.vertex_index >= 0 && 3 * idx.vertex_index + 2 < attrib.vertices.size()) {
                        vertices.push_back(attrib.vertices[3 * idx.vertex_index]);
                        vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                        vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});
                    }

                    // Color (white default)
                    vertices.insert(vertices.end(), {1.0f, 1.0f, 1.0f});

                    // TexCoord
                    if (idx.texcoord_index >= 0 && 2 * idx.texcoord_index + 1 < attrib.texcoords.size()) {
                        vertices.push_back(attrib.texcoords[2 * idx.texcoord_index]);
                        vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 0.0f});
                    }

                    // Normal
                    if (idx.normal_index >= 0 && 3 * idx.normal_index + 2 < attrib.normals.size()) {
                        vertices.push_back(attrib.normals[3 * idx.normal_index]);
                        vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                        vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
                    } else {
                        vertices.insert(vertices.end(), {0.0f, 1.0f, 0.0f});
                    }
                }

                for (size_t v = 0; v < fv; ++v) {
                    indices.push_back(static_cast<Uint32>(vertex_offset + v));
                }

                vertex_offset += fv;
                index_offset += fv;
            }

            const tinyobj::material_t* mat_ptr = nullptr;
            if (!mesh.material_ids.empty() && mesh.material_ids[0] >= 0 && mesh.material_ids[0] < materials.size()) {
                mat_ptr = &materials[mesh.material_ids[0]];
            }

            CreateMeshComponentOBJ(staticMeshObject, layout, vertices, indices, mat_ptr, base_path);
        }
    }

    // ============================================================================
    // Public API Implementation
    // ============================================================================

    GameObject* Model::Load(std::string_view path, Scene* scene) {
        if (!scene) {
            spdlog::error("Model::Load failed: Scene is invalid");
            return nullptr;
        }

        std::string extension = FileSystem::GetFileExtension(path);

        if (extension == "gltf" || extension == "glb") {
            return LoadGLTF(path, scene);
        } else if (extension == "obj") {
            return LoadOBJ(path, scene);
        } else {
            spdlog::error("Unsupported model format: {}", path);
            return nullptr;
        }
    }

    GameObject* Model::LoadGLTF(std::string_view path, Scene* scene) {
        auto& fs = Engine::GetInstance().GetFileSystem();

        std::vector<char> fileData = fs.LoadAssetFile(path);
        if (fileData.empty()) {
            spdlog::error("Failed to load GLTF/GLB file: {}", path);
            return nullptr;
        }

        std::string fullPath = fs.GetAssetsPath() + std::string(path);

        size_t slash         = path.find_last_of("/\\");
        std::string basePath = (slash == std::string::npos) ? "" : std::string(path.substr(0, slash + 1));

        std::string model_name(path);
        if (slash != std::string::npos) {
            model_name = model_name.substr(slash + 1);
        }

        cgltf_options options{};
        cgltf_data* data = nullptr;

        cgltf_result result = cgltf_parse(&options, fileData.data(), fileData.size(), &data);

        if (result != cgltf_result_success) {
            spdlog::error("Failed to parse GLTF/GLB file: {}", fullPath);
            return nullptr;
        }

        result = cgltf_load_buffers(&options, data, fullPath.c_str());
        if (result != cgltf_result_success) {
            spdlog::warn("Failed to load some GLTF/GLB buffers, but continuing: {}", fullPath);
        }

        result = cgltf_validate(data);
        if (result != cgltf_result_success) {
            spdlog::warn("GLTF/GLB validation warning: {}", fullPath);
        }

        if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
            spdlog::error("No mesh data found in GLTF/GLB: {}", path);
            cgltf_free(data);
            return nullptr;
        }

        GameObject* rootObject = scene->CreateObject(model_name, nullptr);

        // Parse scene graph
        cgltf_scene* gltf_scene = data->scene ? data->scene : &data->scenes[0];
        for (cgltf_size i = 0; i < gltf_scene->nodes_count; ++i) {
            ParseGLTFNode(gltf_scene->nodes[i], rootObject, data, basePath, scene);
        }

        // Load animations (hierarchy animations)
        LoadGLTFAnimations(data, rootObject);

        // Load animations (skeletal animations)
        LoadGLTFSkeleton(data, rootObject);

        cgltf_free(data);
        fileData.clear();
        fileData.shrink_to_fit();

        spdlog::info("Successfully loaded GLTF/GLB model: {}", path);
        return rootObject;
    }

    GameObject* Model::LoadOBJ(std::string_view path, Scene* scene) {
        auto& fs = Engine::GetInstance().GetFileSystem();

        std::string dir(path);
        size_t s = dir.find_last_of("/\\");
        dir      = (s == std::string::npos) ? "" : dir.substr(0, s + 1);

        std::string model_name(path);
        size_t last_slash = model_name.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            model_name = model_name.substr(last_slash + 1);
        }

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        std::string full_path = fs.GetAssetsPath() + std::string(path);
        std::string mtl_dir   = fs.GetAssetsPath() + dir;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, full_path.c_str(), mtl_dir.c_str(), true);

        if (!err.empty()) {
            if (!ret) {
                spdlog::error("OBJ error: {}", err);
                return nullptr;
            } else {
                spdlog::warn("OBJ warning: {}", err);
            }
        }

        if (shapes.empty()) {
            spdlog::error("No StaticMeshes found in OBJ: {}", path);
            return nullptr;
        }

        GameObject* rootObject = scene->CreateObject(model_name, nullptr);
        ParseOBJ(attrib, shapes, materials, rootObject, dir, scene);

        spdlog::info("Successfully loaded OBJ model: {}", path);
        return rootObject;
    }

} // namespace golias
