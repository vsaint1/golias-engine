
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

out vec3 NORMAL;
out vec3 WORLD_POSITION;
out vec2 UV;

uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJECTION;

void main() {
    WORLD_POSITION = vec3(MODEL * vec4(a_pos, 1.0));
    NORMAL = mat3(transpose(inverse(MODEL))) * a_normal;
    UV = a_tex_coord;
    gl_Position = PROJECTION * VIEW * vec4(WORLD_POSITION, 1.0);
}