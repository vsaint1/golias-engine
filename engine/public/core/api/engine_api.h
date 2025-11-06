#pragma once
#include "core/engine.h"


void create_camera_entity(const flecs::world& world, const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0),
                          float fov = 45.0f, float near_plane = 0.1f, float far_plane = 1000.0f);

void create_mesh_entity(const char* name, const char* path, const glm::vec3& position = glm::vec3(0),
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
void create_model_entity(const char* name, const char* path, BlendMode blend_mode = BlendMode::OPAQUE, const glm::vec3& position = glm::vec3(0),
                         const glm::vec3& rotation = glm::vec3(0), const glm::vec3& scale = glm::vec3(1.0f));


void create_model_entity_with_blend(const char* name, const char* path, BlendMode blend_mode, 
                                    const glm::vec3& position = glm::vec3(0),
                                    const glm::vec3& rotation = glm::vec3(0), 
                                    const glm::vec3& scale = glm::vec3(1.0f));

void create_material(const char* name, Material material);
