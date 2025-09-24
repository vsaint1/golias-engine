#pragma once

#include "core/system/logging.h"


// ==============================================================
// ALL COMPONENTS ARE DEFINED HERE, PURE DATA AND SIMPLE LOGICS |
// ==============================================================


struct Transform2D {
    glm::vec2 position = {0, 0};
    glm::vec2 scale    = {1, 1};
    float rotation     = 0;
};


enum class ShapeType { RECTANGLE, TRIANGLE, CIRCLE, LINE, POLYGON };

struct Shape {
    ShapeType type = ShapeType::RECTANGLE;
    glm::vec4 color{1, 1, 1, 1};
    bool filled = true;

    // rectangle/triangle
    glm::vec2 size{32, 32};

    // circle
    float radius = 16;
    int segments = 32;

    // line
    glm::vec2 end{50, 0};

    // polygon
    std::vector<glm::vec2> vertices;
};


struct Script {
    std::string path     = "";
    lua_State* lua_state = nullptr;
    bool ready_called    = false;

    ~Script();
};
