layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texCoord;
layout (location = 2) in vec4 a_color;

out vec2 v_texCoord;
out vec4 v_color;
out vec2 v_position;

uniform mat4 VIEW_PROJECTION;

void main() {
    v_texCoord = a_texCoord;
    v_color = a_color;
    v_position = a_position;
    gl_Position = VIEW_PROJECTION * vec4(a_position, 0.0, 1.0);
}
