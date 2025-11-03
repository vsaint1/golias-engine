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

class Transform2D {
public:
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 scale = glm::vec2(1.0f);
    float rotation = 0.0f;

    glm::vec2 origin = glm::vec2(0.0f);

    Transform2D() = default;

    explicit Transform2D(const glm::vec2& pos, const glm::vec2& scl = glm::vec2(1.0f), float rot = 0.0f)
        : position(pos), scale(scl), rotation(rot) {}

    glm::mat4 get_matrix() const {
        glm::mat4 matrix(1.0f);
        matrix = glm::translate(matrix, glm::vec3(-origin, 0.0f));

        matrix = glm::scale(matrix, glm::vec3(scale, 1.0f));
        matrix = glm::rotate(matrix, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::translate(matrix, glm::vec3(position + origin, 0.0f));

        return matrix;
    }

    void set_rotation_degrees(float degrees) {
        rotation = glm::radians(degrees);
    }

    float get_rotation_degrees() const {
        return glm::degrees(rotation);
    }

    void move(const glm::vec2& offset) {
        position += offset;
    }

    void rotate(float radians) {
        rotation += radians;
    }

    void rotate_degrees(float degrees) {
        rotation += glm::radians(degrees);
    }

    void scale_by(const glm::vec2& factor) {
        scale *= factor;
    }

    void set_origin_center() {
        origin = glm::vec2(0.0f); // For sprites, you might want to set this based on size
    }

    void set_origin(const glm::vec2& new_origin) {
        origin = new_origin;
    }
};



struct Rect2D {
    float x      = 0.0f;
    float y      = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;

    Rect2D() = default;

    bool is_zero() const {
        return x == 0.0f && y == 0.0f && width == 0.0f && height == 0.0f;
    }

    Rect2D(float w, float h) : x(0), y(0), width(w), height(h) {
    }

    Rect2D(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h) {
    }
};

class Camera2D {
public:
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f;
    float zoom = 1.0f;
    glm::vec2 viewport_size = glm::vec2(1.0f);

    glm::mat4 get_view_matrix() const {
        glm::mat4 view(1.0f);
        view = glm::translate(view, glm::vec3(-position, 0.0f));
        view = glm::rotate(view, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
        return view;
    }

    glm::mat4 get_projection_matrix() const {
       
        if (viewport_size.x <= 0.0f || viewport_size.y <= 0.0f) {
            return glm::mat4(1.0f);
        }

        return glm::ortho(0.0f, viewport_size.x, viewport_size.y, 0.0f);
    }


    void move(const glm::vec2& offset) {
        position += offset;
    }

    void rotate(float radians) {
        rotation += radians;
    }

    void zoom_by(float factor) {
        zoom *= factor;
    }

    void set_zoom(float new_zoom) {
        zoom = new_zoom;
    }

    void look_at(const glm::vec2& target) {
        position = target;
    }
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
