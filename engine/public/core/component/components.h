#pragma once
#include "stdafx.h"



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
    Uint32 VAO     = 0;
    Uint32 VBO     = 0;
    Uint32 EBO     = 0;
    int indexCount = 0;
};

class Shader;

struct Material {
    glm::vec3 albedo = glm::vec3(1.0f);
    float metallic   = 0.0f;
    float roughness  = 0.5f;
    float ao         = 1.0f;

    // Emissive properties
    glm::vec3 emissive     = glm::vec3(0.0f);
    float emissiveStrength = 1.0f;

    Uint32 albedoMap    = 0;
    Uint32 metallicMap  = 0;
    Uint32 roughnessMap = 0;
    Uint32 normalMap    = 0;
    Uint32 aoMap        = 0;
    Uint32 emissiveMap  = 0;

    bool useAlbedoMap    = false;
    bool useMetallicMap  = false;
    bool useRoughnessMap = false;
    bool useNormalMap    = false;
    bool useAOMap        = false;
    bool useEmissiveMap  = false;

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
    float view_distance = 1000.f;

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
    float shadowDistance = 50.0f;
    float shadowNear     = 1.0f;
    float shadowFar      = 100.0f;

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
    std::vector<MeshInstance3D> meshes;
    std::vector<Material> materials;
};
