#pragma once

#include "core/io/reflection.h"


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



// ---------------- COMPONENT REFLECTION ----------------
REFLECT_COMPONENT(Transform2D, FIELD(Transform2D, position), FIELD(Transform2D, scale), FIELD(Transform2D, rotation))

REFLECT_COMPONENT(Shape, FIELD(Shape, type), FIELD(Shape, color), FIELD(Shape, filled), FIELD(Shape, size), FIELD(Shape, radius),
                  FIELD(Shape, end), FIELD(Shape, vertices))
