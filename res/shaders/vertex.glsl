#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;

out vec3 v_color;
out vec2 v_texcoord;

uniform mat4 MODEL_MATRIX;
uniform mat4 PROJECTION_MATRIX;
uniform mat4 VIEW_MATRIX;

void main() {
    v_color = a_color;
    v_texcoord = a_texcoord;
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(a_pos, 1.0);
}