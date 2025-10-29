#pragma once
#include "stdafx.h"
#include "core/renderer/base_struct.h"


/*!
 * @brief Represents material properties for 3D rendering.
 * @ingroup Components
 */
struct Transform3D {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 get_matrix() const;
};

/*!
 * @brief Represents a physics body for 2D or 3D physics simulations.
 * Jolt Physics -> BodyID
 * Box2D -> b2BodyId
 * @ingroup Components
 */
struct PhysicsBody {
    Uint32 id = 0;
};


struct MeshInstance3D {
    std::string name;

    std::shared_ptr<GpuBuffer> vertex_buffer;
    std::shared_ptr<GpuBuffer> index_buffer;
    std::shared_ptr<GpuVertexLayout> vertex_layout;

    int index_count = 0;
};

class Shader;

struct Material {
    glm::vec3 albedo = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(0.0f);
    float metallic   = 0.0f;
    float roughness  = 0.2f;
    float ao         = 1.0f; /// Ambient Occlusion

    glm::vec3 emissive      = glm::vec3(0.0f);
    float emissive_strength = 1.0f;
    float ior              = 1.0f; // Index of Refraction

    Uint32 albedo_map    = 0;
    Uint32 specular_map = 0;
    Uint32 metallic_map  = 0;
    Uint32 roughness_map = 0;
    Uint32 normal_map    = 0;
    Uint32 ao_map        = 0;
    Uint32 emissive_map  = 0;

    bool use_albedo_map    = false;
    bool use_specular_map = false;
    bool use_metallic_map  = false;
    bool use_roughness_map = false;
    bool use_normal_map    = false;
    bool use_ao_map        = false;
    bool use_emissive_map  = false;

    void bind(Shader* shader) const;
};

/*!

    @brief 3D Camera
    - Position
    - Zoom
    - Rotation
    - View matrix
    - Projection matrix

    @ingroup Components
    @version  0.0.1

*/
struct Camera3D {
    float yaw           = -90.0f; // Z
    float pitch         = 0.0f;
    float fov           = 45.0f;
    float speed         = 5.0f;
    float view_distance = 5000.f;

    explicit Camera3D() {
        update_vectors();
    }

    glm::mat4 get_view(const Transform3D& transform) const;

    glm::mat4 get_projection(int w, int h) const;

    void move_forward(Transform3D& transform, float dt);

    void move_backward(Transform3D& transform, float dt);

    void move_left(Transform3D& transform, float dt);

    void move_right(Transform3D& transform, float dt);

    void look_at(float xoffset, float yoffset, float sensitivity = 0.1f);

    void zoom(float yoffset);

private:
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{};
    glm::vec3 right{};
    glm::vec3 world_up{0.0f, 1.0f, 0.0f};

    void update_vectors();
};


struct DirectionalLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity  = 1.0f;
    bool castShadows = true;

    // Shadow projection
    float shadowDistance = 100.0f;
    float shadowNear     = 1.0f;
    float shadowFar      = 200.0f;

    glm::mat4 get_light_space_matrix(glm::mat4 camera_view, glm::mat4 camera_proj) const;

    glm::mat4 get_light_space_matrix() const;
};

struct SpotLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity   = 1.0f;
    float cutOff      = 12.5f;
    float outerCutOff = 17.5f;
};


struct Model {
    std::string path;
    std::vector<MeshInstance3D> meshes;
    std::vector<Material> materials;
};

struct MeshRef {
    const MeshInstance3D* mesh;
};

struct MaterialRef {
    const Material* material;
};
