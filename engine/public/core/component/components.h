#pragma once

#include "core/system/logging.h"


// ==============================================================
// ALL COMPONENTS ARE DEFINED HERE, PURE DATA AND SIMPLE LOGICS |
// ==============================================================

/*!
 * @brief Represents a 2D transformation position, scale, and rotation.
 */
struct Transform2D {
    glm::vec2 position = {0, 0};
    glm::vec2 scale    = {1, 1};
    float rotation     = 0;

    glm::vec2 world_position = {0, 0};
    glm::vec2 world_scale    = {1, 1};
    float world_rotation     = 0;
};

/*!
 * @brief Different types of shapes that can be rendered.
 */
enum class ShapeType { RECTANGLE, TRIANGLE, CIRCLE, LINE, POLYGON };

/*!
 * @brief Represents a shape with its properties for rendering.
 */
struct Shape {
    ShapeType type = ShapeType::RECTANGLE; /// Type of the shape
    glm::vec4 color{1, 1, 1, 1};
    bool filled = true;

    glm::vec2 size{32, 32}; /// size for `rectangle` and `triangle`

    /// radius/segments for `circle`
    float radius = 16;

    /// end point for `line`
    glm::vec2 end{50, 0};

    /// vertices for `polygon`
    std::vector<glm::vec2> vertices;
};

/*!
 * @brief Represents a script component that holds Lua script information.
 */
struct Script {
    std::string path     = "";
    lua_State* lua_state = nullptr;
    bool ready_called    = false;

    ~Script();
};

/*!
 * @brief Represents a label component for rendering text.
 */
struct Label2D {
    std::string text      = "";
    glm::vec4 color       = {1, 1, 1, 1};
    std::string font_name = "default";
    int font_size         = 16;
};


/*!
    * @brief Represents a 2D sprite component for rendering textures.
*/
struct Sprite2D {
    std::string texture_name = "";
    glm::vec4 source         = {0, 0, 64, 64}; // x, y, w, h
    glm::vec4 color          = {1, 1, 1, 1};
    bool flip_h              = false;
    bool flip_v              = false;
};

struct Scene {};

struct SceneRoot {};

struct ActiveScene {};

struct Alive {}; // Marks entities that are alive (children of active scene)

struct SceneChangeRequest {
    std::string name;
};


struct Transform3D {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 get_model_matrix() const {
        glm::mat4 m(1.0f);
        m = glm::translate(m, position);
        m = glm::rotate(m, rotation.x, glm::vec3(1, 0, 0));
        m = glm::rotate(m, rotation.y, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rotation.z, glm::vec3(0, 0, 1));
        m = glm::scale(m, scale);
        return m;
    }
};


// TODO: remove this from components
struct Mesh {
    Uint32 vao              = 0;
    Uint32 vbo              = 0;
    Uint32 texture_id       = 0;
    bool has_texture        = false;
    size_t vertex_count     = 0;
    glm::vec3 diffuse_color = glm::vec3(0.5f);

    ~Mesh();
};


struct Model {
    std::string path;
    std::vector<Mesh> meshes;

    ~Model();
};


enum MOVEMENT { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera3D {
public:
    glm::vec3 position{0.0f, 1.5f, 4.0f};
    float yaw           = -90.0f; // Z
    float pitch         = 0.0f;
    float fov           = 45.0f;
    float speed         = 5.0f;
    float view_distance = 1000.f;

    explicit Camera3D(glm::vec3 pos = {0, 1.5f, 4.0f}) : position(pos) {
        update_vectors();
    }

    glm::mat4 get_view() const;

    glm::mat4 get_projection(int w, int h) const;

    void move_forward(float dt);

    void move_backward(float dt);

    void move_left(float dt);

    void move_right(float dt);

    void look_at(float xoffset, float yoffset, float sensitivity = 0.1f);

    void zoom(float yoffset);

private:
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{};
    glm::vec3 right{};
    glm::vec3 world_up{0.0f, 1.0f, 0.0f};

    void update_vectors();
};


inline void serialize_components(flecs::world& ecs) {

    ecs.component<Scene>();

    ecs.component<SceneRoot>();

    ecs.component<ActiveScene>().add(flecs::Exclusive);

    ecs.component<Model>();

    ecs.component<glm::vec2>().member<float>("x").member<float>("y");
    ecs.component<glm::vec4>().member<float>("x").member<float>("y").member<float>("z").member<float>("w");
    ecs.component<std::string>()
        .opaque(flecs::String)
        .serialize([](const flecs::serializer* s, const std::string* data) {
            const char* str = data->c_str();
            return s->value(flecs::String, &str);
        })
        .assign_string([](std::string* data, const char* value) { *data = value; });

    ecs.component<Transform2D>().member<glm::vec2>("position").member<glm::vec2>("scale").member<float>("rotation");

    ecs.component<ShapeType>()
        .constant("RECTANGLE", ShapeType::RECTANGLE)
        .constant("TRIANGLE", ShapeType::TRIANGLE)
        .constant("CIRCLE", ShapeType::CIRCLE)
        .constant("LINE", ShapeType::LINE)
        .constant("POLYGON", ShapeType::POLYGON);

    ecs.component<Shape>()
        .member<ShapeType>("type")
        .member<glm::vec4>("color")
        .member<bool>("filled")
        .member<glm::vec2>("size")
        .member<float>("radius")
        .member<glm::vec2>("end");

    ecs.component<Label2D>()
        .member<std::string>("text")
        .member<glm::vec4>("color")
        .member<std::string>("font_name")
        .member<int>("font_size");
}
