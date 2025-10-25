#pragma once
#include "stdafx.h"



// ============================================================================
// COMPONENTS
// ============================================================================

struct Transform3D {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 get_matrix() const;
};

struct MeshInstance3D {
    Uint32 VAO     = 0;
    Uint32 VBO     = 0;
    Uint32 EBO     = 0;
    int indexCount = 0;
};

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
};

struct Camera3D {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 front    = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov          = 45.0f;
    float nearPlane    = 0.1f;
    float farPlane     = 1000.0f;

    // Camera movement
    float yaw              = -90.0f;
    float pitch            = 0.0f;
    float movementSpeed    = 5.0f;
    float mouseSensitivity = 0.1f;

    void update_vectors();

    glm::mat4 get_view_matrix() const;

    glm::mat4 get_projection_matrix(float aspect) const;
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
