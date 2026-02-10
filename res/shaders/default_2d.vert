layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

out vec2 v_uv;
out vec4 v_color;

uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;

void main() {
    v_uv = a_uv;
    v_color = a_color;
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * vec4(a_pos, 1.0);
}