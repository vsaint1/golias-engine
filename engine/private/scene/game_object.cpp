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
#include "scene/3d/animation_component.h"
#include <stb_image.h>

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

        // Vertex layout utilities
        VertexLayout CreateStandardVertexLayout() {
            VertexLayout layout;
            Uint32 offset = 0;

            // Position at location 0
            layout.elements.push_back({0, 3, EDataType::FLOAT, false, offset});
            offset += 12;

            // Color at location 1
            layout.elements.push_back({1, 3, EDataType::FLOAT, false, offset});
            offset += 12;

            // Texcoord at location 2
            layout.elements.push_back({2, 2, EDataType::FLOAT, false, offset});
            offset += 8;

            // Normal at location 3
            layout.elements.push_back({3, 3, EDataType::FLOAT, false, offset});
            offset += 12;

            layout.stride = offset;
            return layout;
        }

        // Vertex data interleaving utilities
        struct VertexAttributeData {
            std::vector<float> positions;
            std::vector<float> colors;
            std::vector<float> texcoords;
            std::vector<float> normals;
            size_t vertexCount = 0;

            const cgltf_accessor* positionAccessor = nullptr;
            const cgltf_accessor* colorAccessor    = nullptr;
            const cgltf_accessor* texcoordAccessor = nullptr;
            const cgltf_accessor* normalAccessor   = nullptr;
        };

        bool ExtractGLTFVertexData(const cgltf_primitive* prim, VertexAttributeData& data) {
            // Find accessors for each attribute type
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
                default:
                    break;
                }
            }

            if (data.vertexCount == 0) {
                return false;
            }

            // Extract data from accessors
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

            return true;
        }

        std::vector<float> InterleaveVertexData(const VertexAttributeData& data) {
            std::vector<float> vertices;
            vertices.reserve(data.vertexCount * 11); // 3 pos + 3 color + 2 texcoord + 3 normal

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
            }

            return vertices;
        }

        bool LoadGLTF(const std::string_view path, cgltf_data*& outData, std::string& outBasePath) {
            auto& fs = Engine::GetInstance().GetFileSystem();

            std::string fullPath = fs.GetAssetsPath() + std::string(path);

            size_t s    = path.find_last_of("/\\");
            outBasePath = (s == std::string::npos) ? "" : std::string(path.substr(0, s + 1));

            cgltf_options options = {};
            memset(&options, 0, sizeof(cgltf_options));

            cgltf_result result = cgltf_parse_file(&options, fullPath.c_str(), &outData);

            if (result != cgltf_result_success) {
                spdlog::error("Failed to parse GLTF/GLB file: {}", fullPath);
                return false;
            }

            result = cgltf_load_buffers(&options, outData, fullPath.c_str());

            if (result != cgltf_result_success) {
                spdlog::warn("Failed to load some GLTF/GLB buffers, but continuing: {}", fullPath);
            }

            result = cgltf_validate(outData);
            if (result != cgltf_result_success) {
                spdlog::warn("GLTF/GLB validation warning: {}", fullPath);
            }

            return true;
        }

        void FreeGLTF(cgltf_data* data) {
            if (data) {
                cgltf_free(data);
            }
        }

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

        void LoadGLTFAnimations(cgltf_data* data, GameObject* rootObject) {
            std::vector<std::shared_ptr<AnimationClip>> clips;

            for (cgltf_size ai = 0; ai < data->animations_count; ++ai) {
                auto& anim     = data->animations[ai];
                auto clip      = std::make_shared<AnimationClip>();
                clip->name     = anim.name ? anim.name : "no_name";
                clip->duration = 0.0f;

                spdlog::info("Processing GLTF Animation clip: {} with {} channels", clip->name, anim.channels_count);

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

                spdlog::info("Loaded {} Animation clips from GLTF file", data->animations_count);
            }
        }


        void CreateMeshComponentGLTF(GameObject* gameObject,
                                     const VertexLayout& layout,
                                     const std::vector<float>& vertices,
                                     const std::vector<Uint32>& indices,
                                     cgltf_material* gltf_material,
                                     const std::string& base_path) {
            auto& engine          = Engine::GetInstance();
            auto rd               = engine.GetRenderingDevice();
            auto& texture_manager = engine.GetTextureManager();

            std::shared_ptr<Mesh> mesh         = rd->CreateMeshFromData(layout, vertices, indices);
            std::shared_ptr<Material> material = std::make_shared<Material>();
            std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
            material->SetShader(shader);

            // Default material parameters
            glm::vec4 base_color(1.0f);
            float metallic  = 0.0f;
            float roughness = 1.0f;
            glm::vec3 emissive(0.0f);

            // Initialize texture flags
            material->SetParameter("HAS_ALBEDO", 0);
            material->SetParameter("HAS_METALLIC", 0);
            material->SetParameter("HAS_ROUGHNESS", 0);
            material->SetParameter("HAS_NORMAL", 0);
            material->SetParameter("HAS_AO", 0);
            material->SetParameter("HAS_EMISSIVE", 0);


            auto LoadGLTFTexture = [&](cgltf_texture* texture, const std::string& fallback_name = "") -> std::shared_ptr<Texture2D> {
                if (!texture || !texture->image) {
                    return nullptr;
                }

                cgltf_image* image = texture->image;

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
                        unsigned char* image_data =
                            stbi_load_from_memory(decoded.data(), static_cast<int>(decoded.size()), &width, &height, &channels, 0);

                        if (!image_data) {
                            spdlog::warn("Failed to decode data URI texture {}: {}", fallback_name, stbi_failure_reason());
                            return nullptr;
                        }

                        std::stringstream embedded_id;
                        embedded_id << "datauri://";

                        if (!fallback_name.empty()) {
                            embedded_id << fallback_name;
                        } else {
                            uint32_t hash = 0;
                            for (size_t i = 0; i < std::min(decoded.size(), static_cast<size_t>(1024)); ++i) {
                                hash = ((hash << 5) + hash) + decoded[i];
                            }
                            embedded_id << "texture_" << std::hex << hash;
                        }

                        embedded_id << "_" << width << "x" << height << "_c" << channels;
                        std::string embedded_path = embedded_id.str();

                        try {
                            ETextureFormat format;
                            if (channels == 1) {
                                format = ETextureFormat::RED;
                            } else if (channels == 2) {
                                format = ETextureFormat::RG;
                            } else if (channels == 3) {
                                format = ETextureFormat::RGB;
                            } else if (channels == 4) {
                                format = ETextureFormat::RGBA;
                            } else {
                                stbi_image_free(image_data);
                                spdlog::warn("Unsupported channel count {} for {}", channels, fallback_name);
                                return nullptr;
                            }

                            std::shared_ptr<Texture2D> loaded_texture =
                                texture_manager.EnsureTexture2D(embedded_path, width, height, format, image_data);


                            if (loaded_texture) {
                                spdlog::debug("Successfully created data URI texture: {} ({}x{} {} channels)",
                                              embedded_path,
                                              width,
                                              height,
                                              channels);
                                return loaded_texture;
                            } else {
                                spdlog::warn("Texture manager returned null for data URI texture: {}", fallback_name);
                            }
                        } catch (const std::exception& e) {
                            stbi_image_free(image_data);
                            spdlog::warn("Failed to create texture from data URI {}: {}", fallback_name, e.what());
                        }

                        return nullptr;
                    } else {
                        std::string tex_path = base_path + image->uri;
                        try {
                            auto loaded_texture = texture_manager.EnsureTexture2D(tex_path);
                            if (loaded_texture) {
                                spdlog::debug("Loaded external texture: {}", tex_path);
                                return loaded_texture;
                            }
                        } catch (const std::exception& e) {
                            spdlog::warn("Failed to load external texture {}: {}", tex_path, e.what());
                        }
                        return nullptr;
                    }
                }

                if (image->buffer_view) {
                    spdlog::info("Loading embedded texture from buffer view: {} ({} bytes)",
                                 fallback_name.empty() ? "unnamed" : fallback_name,
                                 image->buffer_view->size);

                    const uint8_t* data = static_cast<const uint8_t*>(image->buffer_view->buffer->data) + image->buffer_view->offset;
                    size_t size         = image->buffer_view->size;

                    int width, height, channels;

                    if (!stbi_info_from_memory(data, static_cast<int>(size), &width, &height, &channels)) {
                        spdlog::warn("STBI cannot identify embedded image format for {}", fallback_name);
                        return nullptr;
                    }

                    unsigned char* image_data = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 0);

                    if (!image_data) {
                        spdlog::warn("Failed to decode embedded texture {}: {}", fallback_name, stbi_failure_reason());
                        return nullptr;
                    }

                    std::stringstream embedded_id;
                    embedded_id << "embedded://";

                    if (!base_path.empty()) {
                        size_t last_slash = base_path.find_last_of("/\\");
                        if (last_slash != std::string::npos && last_slash + 1 < base_path.length()) {
                            std::string model_name = base_path.substr(last_slash + 1);
                            size_t dot_pos         = model_name.find_last_of('.');
                            if (dot_pos != std::string::npos) {
                                model_name = model_name.substr(0, dot_pos);
                            }
                            embedded_id << model_name << "/";
                        }
                    }

                    if (!fallback_name.empty()) {
                        embedded_id << fallback_name;
                    } else {

                        uint32_t hash = 0;
                        for (size_t i = 0; i < std::min(size, static_cast<size_t>(1024)); ++i) {
                            hash = ((hash << 5) + hash) + data[i];
                        }
                        embedded_id << "texture_" << std::hex << hash;
                    }

                    embedded_id << "_" << width << "x" << height << "_c" << channels;
                    std::string embedded_path = embedded_id.str();
                    spdlog::debug("Embedded texture ID: {}", embedded_path);

                    try {
                        ETextureFormat format;
                        if (channels == 1) {
                            format = ETextureFormat::RED;
                        } else if (channels == 2) {
                            format = ETextureFormat::RG;
                        } else if (channels == 3) {
                            format = ETextureFormat::RGB;
                        } else if (channels == 4) {
                            format = ETextureFormat::RGBA;
                        } else {
                            stbi_image_free(image_data);
                            spdlog::warn("Unsupported channel count {} for {}", channels, fallback_name);
                            return nullptr;
                        }

                        std::shared_ptr<Texture2D> loaded_texture =
                            texture_manager.EnsureTexture2D(embedded_path, width, height, format, image_data);


                        if (loaded_texture) {
                            spdlog::debug(
                                "Successfully created embedded texture: {} ({}x{} {} channels)", embedded_path, width, height, channels);
                            return loaded_texture;
                        } else {
                            spdlog::warn("Texture manager returned null for embedded texture: {}", fallback_name);
                        }
                    } catch (const std::exception& e) {
                        stbi_image_free(image_data);
                        spdlog::warn("Failed to create texture from embedded data {}: {}", fallback_name, e.what());
                    }

                    return nullptr;
                }

                spdlog::warn("Texture has neither URI nor buffer_view: {}", fallback_name);
                return nullptr;
            };


            if (gltf_material && gltf_material->has_pbr_metallic_roughness) {
                auto& pbr = gltf_material->pbr_metallic_roughness;
                base_color =
                    glm::vec4(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3]);
                metallic  = pbr.metallic_factor;
                roughness = pbr.roughness_factor;

                // Albedo texture
                if (pbr.base_color_texture.texture) {
                    std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_albedo" : "albedo";
                    auto texture         = LoadGLTFTexture(pbr.base_color_texture.texture, tex_name);
                    if (texture) {
                        material->SetParameter("ALBEDO_TEXTURE", texture);
                        material->SetParameter("HAS_ALBEDO", 1);
                        spdlog::debug("Set albedo texture for material: {}", gltf_material->name ? gltf_material->name : "unnamed");
                    } else if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
                        spdlog::debug("Albedo texture found but not loaded");
                    }
                }

                // Metallic-Roughness texture
                if (pbr.metallic_roughness_texture.texture) {
                    std::string tex_name =
                        gltf_material->name ? std::string(gltf_material->name) + "_metallic_roughness" : "metallic_roughness";
                    auto texture = LoadGLTFTexture(pbr.metallic_roughness_texture.texture, tex_name);
                    if (texture) {
                        material->SetParameter("METALLIC_TEXTURE", texture);
                        material->SetParameter("ROUGHNESS_TEXTURE", texture);
                        material->SetParameter("HAS_METALLIC", 1);
                        material->SetParameter("HAS_ROUGHNESS", 1);
                        spdlog::debug("Set metallic-roughness texture for material: {}",
                                      gltf_material->name ? gltf_material->name : "unnamed");
                    } else if (pbr.metallic_roughness_texture.texture && pbr.metallic_roughness_texture.texture->image) {
                        spdlog::debug("Metallic-roughness texture found but not loaded");
                    }
                }
            }

            // Normal map
            if (gltf_material && gltf_material->normal_texture.texture) {
                std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_normal" : "normal";
                auto texture         = LoadGLTFTexture(gltf_material->normal_texture.texture, tex_name);
                if (texture) {
                    material->SetParameter("NORMAL_TEXTURE", texture);
                    material->SetParameter("HAS_NORMAL", 1);
                    spdlog::debug("Set normal texture for material: {}", gltf_material->name ? gltf_material->name : "unnamed");
                } else if (gltf_material->normal_texture.texture && gltf_material->normal_texture.texture->image) {
                    spdlog::debug("Normal texture found but not loaded");
                }
            }

            // Occlusion map
            if (gltf_material && gltf_material->occlusion_texture.texture) {
                std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_occlusion" : "occlusion";
                auto texture         = LoadGLTFTexture(gltf_material->occlusion_texture.texture, tex_name);
                if (texture) {
                    material->SetParameter("AO_TEXTURE", texture);
                    material->SetParameter("HAS_AO", 1);
                    spdlog::debug("Set occlusion texture for material: {}", gltf_material->name ? gltf_material->name : "unnamed");
                } else if (gltf_material->occlusion_texture.texture && gltf_material->occlusion_texture.texture->image) {
                    spdlog::debug("Occlusion texture found but not loaded");
                }
            }

            // Emissive
            if (gltf_material && gltf_material->has_emissive_strength) {
                emissive =
                    glm::vec3(gltf_material->emissive_factor[0], gltf_material->emissive_factor[1], gltf_material->emissive_factor[2]);

                if (gltf_material->emissive_texture.texture) {
                    std::string tex_name = gltf_material->name ? std::string(gltf_material->name) + "_emissive" : "emissive";
                    auto texture         = LoadGLTFTexture(gltf_material->emissive_texture.texture, tex_name);
                    if (texture) {
                        material->SetParameter("EMISSIVE_TEXTURE", texture);
                        material->SetParameter("HAS_EMISSIVE", 1);
                        spdlog::debug("Set emissive texture for material: {}", gltf_material->name ? gltf_material->name : "unnamed");
                    } else if (gltf_material->emissive_texture.texture && gltf_material->emissive_texture.texture->image) {
                        spdlog::debug("Emissive texture found but not loaded");
                    }
                }
            }

            material->SetParameter("MODULATE", base_color);
            material->SetParameter("METALLIC_FACTOR", metallic);
            material->SetParameter("ROUGHNESS_FACTOR", roughness);
            material->SetParameter("EMISSIVE_FACTOR", emissive);

            gameObject->AddComponent(new MeshComponent(mesh, material));

            if (gltf_material && gltf_material->name) {
                spdlog::debug(
                    "GLTF Material: {} with {} vertices and {} indices", gltf_material->name, vertices.size() / 11, indices.size());
            }
        }


        void CreateMeshComponentOBJ(GameObject* gameObject,
                                    const VertexLayout& layout,
                                    const std::vector<float>& vertices,
                                    const std::vector<Uint32>& indices,
                                    const tinyobj::material_t* obj_material,
                                    const std::string& base_path) {
            auto& engine = Engine::GetInstance();
            auto rd      = engine.GetRenderingDevice();

            std::shared_ptr<Mesh> mesh         = rd->CreateMeshFromData(layout, vertices, indices);
            std::shared_ptr<Material> material = std::make_shared<Material>();
            std::shared_ptr<Shader> shader     = rd->GetDefaultShader3D();
            material->SetShader(shader);

            // Initialize texture flags
            material->SetParameter("HAS_ALBEDO", 0);
            material->SetParameter("HAS_METALLIC", 0);
            material->SetParameter("HAS_ROUGHNESS", 0);
            material->SetParameter("HAS_NORMAL", 0);
            material->SetParameter("HAS_AO", 0);
            material->SetParameter("HAS_EMISSIVE", 0);

            glm::vec4 base_color(1.0f);

            if (obj_material) {
                base_color = glm::vec4(obj_material->diffuse[0], obj_material->diffuse[1], obj_material->diffuse[2], 1.0f);

                // Diffuse texture
                if (!obj_material->diffuse_texname.empty()) {
                    std::string tex_path = base_path + obj_material->diffuse_texname;
                    auto texture         = Engine::GetInstance().GetTextureManager().EnsureTexture2D(tex_path);
                    if (texture) {
                        material->SetParameter("ALBEDO_TEXTURE", texture);
                        material->SetParameter("HAS_ALBEDO", 1);
                    }
                }

                // Normal map
                if (!obj_material->normal_texname.empty()) {
                    std::string tex_path = base_path + obj_material->normal_texname;
                    auto texture         = Engine::GetInstance().GetTextureManager().EnsureTexture2D(tex_path);
                    if (texture) {
                        material->SetParameter("NORMAL_TEXTURE", texture);
                        material->SetParameter("HAS_NORMAL", 1);
                    }
                }

                float roughness = 1.0f - (obj_material->shininess / 1000.0f);
                material->SetParameter("ROUGHNESS_FACTOR", glm::clamp(roughness, 0.0f, 1.0f));
            }

            material->SetParameter("MODULATE", base_color);
            material->SetParameter("METALLIC_FACTOR", 0.0f);

            gameObject->AddComponent(new MeshComponent(mesh, material));
        }

        // Scene graph parsing
        void ParseGLTFNode(cgltf_node* node, GameObject* parent, cgltf_data* data, const std::string& base_path, Scene* pScene) {


            std::string node_name  = node->name ? node->name : "Node";
            GameObject* nodeObject = pScene->CreateObject(node_name, parent);

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
                    VertexLayout layout         = CreateStandardVertexLayout();

                    // Extract indices
                    std::vector<Uint32> indices;
                    EDataType index_type;
                    if (prim->indices) {
                        ExtractIndexData(prim->indices, indices, index_type);
                    }

                    CreateMeshComponentGLTF(nodeObject, layout, vertices, indices, prim->material, base_path);
                }
            }

            // Recursively process children
            for (cgltf_size c = 0; c < node->children_count; ++c) {
                ParseGLTFNode(node->children[c], nodeObject, data, base_path, pScene);
            }
        }

        void ParseOBJ(const tinyobj::attrib_t& attrib,
                      const std::vector<tinyobj::shape_t>& shapes,
                      const std::vector<tinyobj::material_t>& materials,
                      GameObject* rootObject,
                      const std::string& base_path,
                      Scene* pScene) {
            auto& engine = Engine::GetInstance();


            for (const auto& shape : shapes) {
                GameObject* shapeObject = pScene->CreateObject(shape.name.empty() ? "Shape" : shape.name, rootObject);
                const auto& mesh        = shape.mesh;

                VertexLayout layout = CreateStandardVertexLayout();
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

    } // anonymous namespace

    GameObject* GameObject::FindChildByName(const std::string_view pName) const {
        if (name == pName) {
            return const_cast<GameObject*>(this);
        }

        for (const auto& child : children) {
            if (child->GetName() == pName) {
                return child.get();
            }
            GameObject* found = child->FindChildByName(pName);
            if (found) {
                return found;
            }
        }

        return nullptr;
    }

    void GameObject::AddComponent(Component* pComponent) {
        if (!pComponent) {
            return;
        }

        components.emplace_back(pComponent);
        pComponent->SetOwner(this);
        pComponent->Start();
        spdlog::info("GameObject::AddComponent added Component: {} to GameObject: {}", typeid(*pComponent).name(), typeid(*this).name());
    }

    void GameObject::LoadProperties(const nlohmann::json& json) {
    }

    void GameObject::Start() {
    }

    void GameObject::Update(float deltaTime) {
        if (!IsActive()) {
            return;
        }

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
        isAlive = false;
    }

    bool GameObject::IsAlive() const {
        return isAlive;
    }

    glm::vec3 GameObject::GetWorldPosition() const {

        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        return glm::vec3(hom) / hom.w;
    }


    glm::vec3 GameObject::GetPosition() const {
        return position;
    }

    void GameObject::SetPosition(const glm::vec3& pos) {
        position = pos;
    }

    void GameObject::SetWorldPosition(const glm::vec3& pos) {
        if (parent) {
            glm::mat4 parentWorldTransform = parent->GetWorldTransform();
            glm::mat4 inverseParentWorld   = glm::inverse(parentWorldTransform);
            glm::vec4 localPos             = inverseParentWorld * glm::vec4(pos, 1.0f);
            SetPosition(glm::vec3(localPos) / localPos.w);
        } else {
            SetPosition(pos);
        }
    }


    glm::quat GameObject::GetWorldRotation() const {
        if (parent) {
            return parent->GetWorldRotation() * rotation;
        } else {
            return rotation;
        }
    }
    glm::quat GameObject::GetRotation() const {
        return rotation;
    }

    void GameObject::SetWorldRotation(const glm::quat& rot) {
        if (parent) {
            glm::quat parentWorldRotation = parent->GetWorldRotation();
            SetRotation(glm::inverse(parentWorldRotation) * rot);
        } else {
            SetRotation(rot);
        }
    }

    void GameObject::SetRotation(const glm::quat& rot) {
        rotation = rot;
    }

    glm::vec3 GameObject::GetScale() const {
        return scale;
    }

    void GameObject::SetScale(const glm::vec3& value) {
        scale = value;
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


    GameObject* GameObject::LoadModel(const std::string_view pPath, Scene* pScene) {
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

        if (!pScene) {
            spdlog::error("GameObject::LoadModel failed: Scene is invalid");
            return nullptr;
        }

        GameObject* rootObject = pScene->CreateObject(model_name, nullptr);

        if (extension == "gltf" || extension == "glb") {
            cgltf_data* data = nullptr;
            std::string base_path;

            if (!LoadGLTF(pPath, data, base_path)) {
                rootObject->Destroy();
                return nullptr;
            }

            if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
                spdlog::error("No mesh data found in GLTF/GLB: {}", pPath);
                FreeGLTF(data);
                rootObject->Destroy();
                return nullptr;
            }

            cgltf_scene* scene = data->scene ? data->scene : &data->scenes[0];
            for (cgltf_size i = 0; i < scene->nodes_count; ++i) {
                ParseGLTFNode(scene->nodes[i], rootObject, data, base_path, pScene);
            }

            // for (cgltf_size i = 0; i < data->textures_count; ++i) {
            //     cgltf_texture* tex = data->textures + i;
            //     spdlog::info("GLTF Texture[{}]: {}", i, tex->name ? tex->name : "no_name");
            // }

            LoadGLTFAnimations(data, rootObject);
            FreeGLTF(data);

            spdlog::info("Successfully loaded {} model: {}", extension == "glb" ? "GLB" : "GLTF", pPath);

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

            ParseOBJ(attrib, shapes, materials, rootObject, dir, pScene);
            spdlog::info("Successfully loaded OBJ model: {}", pPath);

        } else {
            spdlog::error("Unsupported model format: {}", pPath);
            rootObject->Destroy();
            return nullptr;
        }

        return rootObject;
    }

    bool GameObject::IsActive() const {
        return isActive;
    }

    void GameObject::SetActive(bool active) {
        isActive = active;
    }

} // namespace golias
