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
 * @brief Creates a 2D rectangle entity
 * @param name The name of the entity
 * @param position Position in 2D space
 * @param size Size of the rectangle
 * @param color Color of the rectangle
 * @param filled Whether the rectangle is filled or outlined
 * @return The created GameObject
 */
GameObject create_rectangle_2d_entity(const char* name, const glm::vec2& position = glm::vec2(0),
                                     const glm::vec2& size = glm::vec2(100, 100),
                                     const glm::vec4& color = glm::vec4(1.0f), bool filled = true);

/*!
 * @brief Creates a 2D circle entity
 * @param name The name of the entity
 * @param position Position in 2D space
 * @param radius Radius of the circle
 * @param color Color of the circle
 * @param filled Whether the circle is filled or outlined
 * @param thickness Thickness of the outline (only used when filled = false)
 * @param segments Number of segments for circle smoothness
 * @return The created GameObject
 */
GameObject create_circle_2d_entity(const char* name, const glm::vec2& position = glm::vec2(0),
                                  float radius = 50.0f, const glm::vec4& color = glm::vec4(1.0f),
                                  bool filled = true, float thickness = 1.0f, int segments = 32);

/*!
 * @brief Creates a 2D line entity
 * @param name The name of the entity
 * @param start Start position of the line
 * @param end End position of the line
 * @param color Color of the line
 * @param thickness Thickness of the line
 * @return The created GameObject
 */
GameObject create_line_2d_entity(const char* name, const glm::vec2& start = glm::vec2(0),
                                const glm::vec2& end = glm::vec2(100, 100),
                                const glm::vec4& color = glm::vec4(1.0f), float thickness = 1.0f);

/*!
 * @brief Creates a 2D triangle entity
 * @param name The name of the entity
 * @param p1 First point of the triangle
 * @param p2 Second point of the triangle
 * @param p3 Third point of the triangle
 * @param color Color of the triangle
 * @param filled Whether the triangle is filled
 * @return The created GameObject
 */
GameObject create_triangle_2d_entity(const char* name, const glm::vec2& p1 = glm::vec2(0, 0),
                                    const glm::vec2& p2 = glm::vec2(100, 0),
                                    const glm::vec2& p3 = glm::vec2(50, 100),
                                    const glm::vec4& color = glm::vec4(1.0f), bool filled = true);

/*!
 * @brief Creates a 2D text label entity
 * @param name The name of the entity
 * @param text The text to display
 * @param position Position in 2D space
 * @param color Color of the text
 * @param scale Scale of the text
 * @return The created GameObject
 */
GameObject create_label_2d_entity(const char* name, const std::string& text = "Text",
                                 const glm::vec2& position = glm::vec2(0),
                                 const glm::vec4& color = glm::vec4(1.0f), float scale = 1.0f);

/*!
 * @brief Creates a 2D sprite entity
 * @param name The name of the entity
 * @param texture_path Path to the texture file
 * @param position Position in 2D space
 * @param size Size of the sprite (0 = use texture size)
 * @param color Tint color
 * @return The created GameObject
 */
GameObject create_sprite_2d_entity(const char* name, const std::string& texture_path,
                                  const glm::vec2& position = glm::vec2(0),
                                  const glm::vec2& size = glm::vec2(0),
                                  const glm::vec4& color = glm::vec4(1.0f));



/*!
 * @brief  Creates a material with the given name and properties.
 * @param name
 * @param material
 */
void create_material(const char* name, Material material);
