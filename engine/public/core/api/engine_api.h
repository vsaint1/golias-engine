#pragma once
#include "core/engine.h"


flecs::entity create_directional_light_3d_entity(const char* name, const glm::vec3& direction = glm::vec3(0.0f, -1.0f, 0.0f),
                                        const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, bool cast_shadows = true,
                                        float shadow_distance = 200.0f, float shadow_near = 1.0f, float shadow_far = 500.0f);

flecs::entity create_spot_light_3d_entity(const char* name, const glm::vec3& position = glm::vec3(0),
                                 const glm::vec3& direction = glm::vec3(0, -1.0f, 0), const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
                                 float inner_cutoff = 15.0f, float outer_cutoff = 20.0f, float intensity = 1.0f);


flecs::entity create_camera3d_entity(const char* name, const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0),
                            std::string_view = "MainCamera", float fov = 45.0f, float near_plane = 0.1f, float far_plane = 5000.0f);

flecs::entity create_mesh_3d_entity(const char* name, const char* path, const glm::vec3& position = glm::vec3(0),
                           const glm::vec3& rotation = glm::vec3(0), const glm::vec3& scale = glm::vec3(1.0f),
                           const char* material_tag = "default_material");

/**
 * Create a model entity with custom blend mode for all materials
 * @param name Entity name
 * @param path Model file path
 * @param blend_mode Blend mode to apply to all materials (OPAQUE, TRANSPARENT, ADDITIVE, etc.)
 * @param position Transform position
 * @param rotation Transform rotation
 * @param scale Transform scale
 */
flecs::entity create_model_3d_entity(const char* name, const char* path, BlendMode blend_mode = BlendMode::OPAQUE,
                            const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0),
                            const glm::vec3& scale = glm::vec3(1.0f));

void create_material(const char* name, Material material);
