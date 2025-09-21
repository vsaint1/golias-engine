#pragma once

#include "core/systems/logging_sys.h"

/*! @brief Vertex struct

    @version 0.0.3
*/
struct Vertex {
    glm::vec2 position; /// Position
    glm::vec2 tex_coord; /// Texture coordinates (UV)
    glm::vec4 color; /// Vertex color
};
