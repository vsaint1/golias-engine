
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

// 3,4,5,6 (mat4 = 4 vec4 attributes)
layout (location = 3) in mat4 a_instance_model; // per-instance model matrix

layout(location = 7) in vec3 a_instance_color; // per-instance color

out vec3 NORMAL;
out vec3 WORLD_POSITION;
out vec2 UV;
out vec3 INSTANCE_COLOR;

uniform mat4 VIEW;
uniform mat4 PROJECTION;

void main() {
    WORLD_POSITION = vec3(a_instance_model * vec4(a_pos, 1.0));
    NORMAL = mat3(transpose(inverse(a_instance_model))) * a_normal;
    UV = a_tex_coord;
    gl_Position = PROJECTION * VIEW * vec4(WORLD_POSITION, 1.0);
    INSTANCE_COLOR = a_instance_color;
}