#pragma once
#include "core/renderer/base_struct.h"
#include "stdafx.h"

// win sh1t
#ifdef OPAQUE
    #undef OPAQUE
#endif
#ifdef TRANSPARENT
    #undef TRANSPARENT
#endif

/**
 * @brief Blending modes for materials
 */
enum class BlendMode {
    OPAQUE, // No blending, fully opaque (default)
    TRANSPARENT, // Alpha blending: src_alpha, 1 - src_alpha
    ADDITIVE, // Additive blending: src_alpha, 1
    MULTIPLY, // Multiplicative blending: dst_color, 0
    SCREEN, // Screen blending: 1, 1 - src_color
    OVERLAY // Overlay blending
};

/*!
 * @brief Represents the world transformation matrix for a 3D object.
 * @ingroup Components
 */
struct WorldTransform {
    glm::mat4 matrix = glm::mat4(1.0f);
};

/*!
 * @brief Represents material properties for 3D rendering.
 * @ingroup Components
 */
class Transform3D {
public:
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 get_matrix() const;
};

/*!
    @brief 2D Transform component for position, scale, and rotation.
    @ingroup Components
*/
struct Transform2D {
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 scale    = glm::vec2(1.0f);
    float rotation     = 0.0f;

    glm::vec2 origin = glm::vec2(0.0f);

    Transform2D() = default;

    explicit Transform2D(const glm::vec2& pos, const glm::vec2& scl = glm::vec2(1.0f), float rot = 0.0f)
        : position(pos), scale(scl), rotation(rot) {
    }

    glm::mat3 get_matrix() const;
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

/*!
 * @brief Represents a 3D mesh instance with associated GPU buffers.
 * @ingroup Components
 */
struct MeshInstance3D {
    std::string name;

    std::shared_ptr<GpuBuffer> vertex_buffer       = nullptr;
    std::shared_ptr<GpuBuffer> index_buffer        = nullptr;
    std::shared_ptr<GpuVertexLayout> vertex_layout = nullptr;

    int index_count = 0;
};

class Shader;

/*!
 * @brief Represents material properties for 3D rendering.
 * @ingroup Components
 */
struct Material {
    glm::vec3 albedo   = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(0.0f);
    float metallic     = 0.0f;
    float roughness    = 0.2f;
    float ao           = 1.0f; /// Ambient Occlusion

    glm::vec3 emissive      = glm::vec3(0.0f);
    float emissive_strength = 1.0f;
    float ior               = 1.0f; // Index of Refraction

    BlendMode blend_mode = BlendMode::OPAQUE;

    Uint32 albedo_map    = 0;
    Uint32 specular_map  = 0;
    Uint32 metallic_map  = 0;
    Uint32 roughness_map = 0;
    Uint32 normal_map    = 0;
    Uint32 ao_map        = 0;
    Uint32 emissive_map  = 0;

    bool use_albedo_map    = false;
    bool use_specular_map  = false;
    bool use_metallic_map  = false;
    bool use_roughness_map = false;
    bool use_normal_map    = false;
    bool use_ao_map        = false;
    bool use_emissive_map  = false;

    void bind(Shader* shader) const;

    void update_feature_flags() {
        has_features = 0;
        if (use_albedo_map) {
            has_features |= HAS_ALBEDO_MAP;
        }
        if (use_specular_map) {
            has_features |= HAS_SPECULAR_MAP;
        }
        if (use_metallic_map) {
            has_features |= HAS_METALLIC_MAP;
        }
        if (use_roughness_map) {
            has_features |= HAS_ROUGHNESS_MAP;
        }
        if (use_normal_map) {
            has_features |= HAS_NORMAL_MAP;
        }
        if (use_ao_map) {
            has_features |= HAS_AO_MAP;
        }
        if (use_emissive_map) {
            has_features |= HAS_EMISSIVE_MAP;
        }
    }

    int has_features = 0;
};

struct Tag {
    std::string_view name = "";
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
    glm::vec3 up    = glm::vec3(0.0f);
    glm::vec3 right = glm::vec3(0.0f);
    glm::vec3 world_up{0.0f, 1.0f, 0.0f};

    void update_vectors();
};

/*!
 * @brief Represents a directional light in 3D space.
 * @ingroup Components
 */
struct DirectionalLight3D {
    glm::vec3 direction = {0.0f, -1.0f, 0.0f};
    glm::vec3 color     = glm::vec3(1.0f);
    float intensity     = 1.0f;
    bool castShadows    = true;

    float shadowOrthoSize = 150.0f;
    float shadowNear      = 0.1f;    // Near plane
    float shadowFar       = 5000.0f;  // Far plane


    glm::mat4 get_light_space_matrix(glm::mat4 camera_view, glm::mat4 camera_proj) const;

    glm::mat4 get_light_space_matrix(const glm::vec3& camera_position) const;
};


/*!
 * @brief Represents a spotlight in 3D space.
 * @ingroup Components
 */
struct SpotLight3D {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity   = 1.0f;
    float cutOff      = 12.5f;
    float outerCutOff = 17.5f;
};


/*!
 * @brief Represents the world environment settings for 3D rendering.
 * @ingroup Components
 */
struct WorldEnvironment3D {
    Uint32 texture = 0;

    glm::vec3 color                                = glm::vec3(0.2, 0.3, 0.3);
    std::shared_ptr<GpuBuffer> vertex_buffer       = nullptr;
    std::shared_ptr<GpuBuffer> index_buffer        = nullptr;
    std::shared_ptr<GpuVertexLayout> vertex_layout = nullptr;

    float brightness = 1.0f;
};


/*!
 * @brief 2D Camera for orthographic projection.
 * @ingroup Components
 */
class Camera2D {
public:
    glm::vec2 position      = glm::vec2(0.0f);
    float rotation          = 0.0f;
    float zoom              = 1.0f;
    glm::vec2 viewport_size = glm::vec2(1.0f);

    glm::mat4 get_view_matrix() const;

    glm::mat4 get_projection_matrix() const;

    void move(const glm::vec2& offset);

    void rotate(float radians);

    void zoom_by(float factor);

    void set_zoom(float new_zoom);

    void look_at(const glm::vec2& target);
};

/*!
 * @brief Represents a native script component C++ class instance.
 * @ingroup Components
 */
struct NativeScript {
    void* instance = nullptr;
};

/*!
 * @brief Represents a Lua script component.
 * @ingroup Components
 */
struct LuaScript {
    std::string script_path;
    sol::state_view lua_state;
};

struct SpriteRenderer2D {
    Uint32 texture_id = -1; // TODO: later shared_ptr<Texture2D>
    glm::vec4 color   = glm::vec4(1.0f);
};

/*!
 * @brief Represents a 3D model composed of multiple meshes and materials.
 * @ingroup Components
 */
struct Model {
    std::string path;
    std::vector<MeshInstance3D> meshes;
    std::vector<Material> materials;
};

/*!
 * @brief Reference to a mesh used in rendering.
 * @ingroup Core
 */
struct MeshRef {
    const MeshInstance3D* mesh;
};

/*!
 * @brief Reference to a material used in rendering.
 * @ingroup Core
 */
struct MaterialRef {
    const Material* material;
};

/*!
 * @brief Tags namespace for engine components
 * @ingroup Tags
 */
namespace tags {
    struct Scene {};
    struct ActiveScene {};
    struct MainCamera {};
}

/*!
 * @brief 2D Sprite component for rendering textures
 * @ingroup Components
 */
struct Sprite2D {
    std::string texture_name;
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec2 size = glm::vec2(0.0f); // 0 = use texture size
};

/*!
 * @brief 2D Text Label component
 * @ingroup Components
 */
struct Label2D {
    std::string text;
    glm::vec4 color = glm::vec4(1.0f);
    float scale = 1.0f;
    std::string font = "default";
};

/*!
 * @brief Shape renderer for 2D primitives
 * @ingroup Components
 */
enum class ShapeType { LINE, CIRCLE, RECTANGLE, CAPSULE, TRIANGLE };

struct Shape2D {
    ShapeType type = ShapeType::RECTANGLE;
    glm::vec4 color = glm::vec4(1.0f);
    bool filled = true;
    glm::vec2 size = glm::vec2(1.0f);
    float radius = 10.0f; /// for circles
    float thickness = 1.0f; /// for outlines
};

/*!
 * @brief Follow component for camera following
 * @ingroup Components
 */
struct Follow {
    flecs::entity target;
    glm::vec3 offset = glm::vec3(0.0f);
    float smoothing = 0.1f;
};



struct SceneTag {};


/*!
 * @brief  Represents a game object in the ECS world.
 * @ingroup Core
 */
class GameObject {
public:

    GameObject() = default;

    explicit GameObject(const flecs::world& world) : _id(world.entity()) {
    }

    explicit GameObject(const flecs::entity entity) : _id(entity) {
    }

    Uint32 get_id() const;

    bool is_valid() const;

    const char* get_name() const;

    void set_name(const char* name);

    bool compare_tag(const char* tag) const;

    template <typename T, typename... Args>
    T& add_component(Args&&... args) {
        if constexpr (sizeof...(Args) == 0) {
            return _id.ensure<T>();
        } else if constexpr (sizeof...(Args) == 1 && (std::is_same_v<std::decay_t<Args>, T> && ...)) {
            _id.set<T>(std::forward<Args>(args)...);
            return _id.ensure<T>();
        } else {
            _id.set<T>(T(std::forward<Args>(args)...));
            return _id.ensure<T>();
        }
    }

    template <typename T>
    void add_component() {
        _id.add<T>();
    }

    template <typename T>
    void remove_component() {
        _id.remove<T>();
    }

    template <typename T>
    T* get_component() {
        return const_cast<T*>(_id.try_get<T>());
    }

    void free() const;


    flecs::entity& entity();

    const flecs::entity& entity() const;


    operator flecs::entity() const;

private:
    flecs::entity _id;

};
