#pragma once
#include "core/engine.h"

/*!
 * @brief Creates a 3D directional light entity with the specified properties.
 * @param name The name of the entity.
 * @param direction The direction vector of the light.
 * @param color The color of the light.
 * @param intensity The intensity of the light.
 * @param cast_shadows Whether the light casts shadows. (default: true)
 * @param shadow_near the near plane distance for shadow mapping.
 * @param shadow_far the far plane distance for shadow mapping.
 * @return The created GameObject representing the 3D directional light entity.
 */
GameObject create_directional_light_3d_entity(const char* name, const glm::vec3& direction = glm::vec3(0.0f, -1.0f, 0.0f),
                                              const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, bool cast_shadows = true,
                                              float shadow_distance = 200.0f, float shadow_near = 1.0f, float shadow_far = 500.0f);

/*!
 * @brief Creates a 3D spotlight entity with the specified properties.
 * @param name The name of the entity.
 * @param position The position of the spotlight in 3D space.
 * @param direction The direction vector of the spotlight.
 * @param color The color of the spotlight.
 * @param inner_cutoff  Inner cutoff angle in degrees.
 * @param outer_cutoff Outer cutoff angle in degrees.
 * @param intensity The intensity of the spotlight.
 * @return The created GameObject representing the 3D spotlight entity.
 */
GameObject create_spot_light_3d_entity(const char* name, const glm::vec3& position = glm::vec3(0),
                                       const glm::vec3& direction = glm::vec3(0, -1.0f, 0),
                                       const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f), float inner_cutoff = 15.0f,
                                       float outer_cutoff = 20.0f, float intensity = 1.0f);


/*!
 * @brief  Creates a 3D camera entity with the specified properties.
 * @param name The name of the entity.
 * @param position The position of the camera in 3D space.
 * @param rotation The rotation of the camera in 3D space.
 * @param fov The field of view of the camera in degrees.
 * @param near_plane The near clipping plane distance.
 * @param far_plane The far clipping plane distance.
 * @return The created GameObject representing the 3D camera entity.
 */
GameObject create_camera_3d_entity(const char* name, const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0),
                                   float fov = 45.0f, float near_plane = 0.1f, float far_plane = 5000.0f);

/*!
 * @brief  Creates a 3D mesh entity from the specified file path with given properties.
 * @param name The name of the entity.
 * @param path The file path to the 3D mesh.
 * @param position The position of the mesh in 3D space.
 * @param rotation The rotation of the mesh in 3D space.
 * @param scale The scale of the mesh in 3D space.
 * @param material_tag The material tag to apply to the mesh.
 * @return The created GameObject representing the 3D mesh entity.
 */
GameObject create_mesh_3d_entity(const char* name, const char* path, const glm::vec3& position = glm::vec3(0),
                                 const glm::vec3& rotation = glm::vec3(0), const glm::vec3& scale = glm::vec3(1.0f),
                                 const char* material_tag = "default_material");

/*!
 * @brief  Creates a 3D model entity from the specified file path with given properties.
 * @param name The name of the entity.
 * @param path The file path to the 3D model.
 * @param blend_mode The blending mode for the model's material.
 * @param position The position of the model in 3D space.
 * @param rotation The rotation of the model in 3D space.
 * @param scale The scale of the model in 3D space.
 * @return The created GameObject representing the 3D model entity.
 */
GameObject create_model_3d_entity(const char* name, const char* path, BlendMode blend_mode = BlendMode::OPAQUE,
                                  const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0),
                                  const glm::vec3& scale = glm::vec3(1.0f));


GameObject create_camera_2d_entity(const char* name, const glm::vec2& position = glm::vec2(0), float rotation = 0.0f, float zoom = 1.0f);


/*!
 * @brief  Creates a material with the given name and properties.
 * @param name
 * @param material
 */
void create_material(const char* name, Material material);
