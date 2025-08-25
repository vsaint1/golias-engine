layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coord;
layout (location = 2) in vec4 a_color;

uniform mat4 PROJECTION;
uniform mat4 VIEW;

out vec2 tex_coord;
out vec4 color;

void main() {
    gl_Position = PROJECTION * VIEW * vec4(a_pos, 0.0, 1.0);
    tex_coord = a_tex_coord;
    color = a_color;
}
