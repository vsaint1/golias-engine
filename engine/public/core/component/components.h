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
    std::string text;
    glm::vec2 offset      = {0, 0};
    glm::vec4 color       = {1, 1, 1, 1};
    std::string font_name = "default";
    int font_size         = 16;
};


struct Scene {};

struct SceneRoot {};

struct ActiveScene {};

struct Alive {};            // Marks entities that are alive (children of active scene)

struct SceneChangeRequest {
    std::string name;
};



inline void serialize_components(flecs::world& ecs) {

    ecs.component<Scene>();

    ecs.component<SceneRoot>();
    
    ecs.component<ActiveScene>();

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
        .member<glm::vec2>("offset")
        .member<glm::vec4>("color")
        .member<std::string>("font_name")
        .member<int>("font_size");
}
