#pragma once

#include "servers/rendering/shader_preprocessor.h"


namespace shaders {
    inline std::string get_default_vertex_2d() {
        std::string header = get_shader_header();
        return header + R"(
layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

uniform mat4 VIEW_PROJECTION_MATRIX;

out vec4 vColor;
out vec2 vTexCoord;

void main() {
    vColor = a_color;
    vTexCoord = a_uv;
    gl_Position = VIEW_PROJECTION_MATRIX * vec4(a_vertex, 1.0);
}
)";
    }

   inline  std::string get_default_fragment_2d() {
        std::string header = get_shader_header();
        return header + R"(
in vec4 vColor;
in vec2 vTexCoord;

out vec4 COLOR;

uniform sampler2D TEXTURE;

void main() {
    vec4 texColor = texture(TEXTURE, vTexCoord);

    if (texColor.a < 0.01) {
        discard;
    }

    COLOR = texColor * vColor;
}
)";
    }
} // namespace shaders
