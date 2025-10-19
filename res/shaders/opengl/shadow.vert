layout(location = 0) in vec3 a_position;
layout(location = 3) in mat4 a_instance_model; // per-instance model matrix

uniform mat4 LIGHT_PROJECTION;


void main() {
    vec3 WORLD_POSITION = vec3(a_instance_model * vec4(a_position, 1.0));
    gl_Position = LIGHT_PROJECTION * vec4(WORLD_POSITION, 1.0);
}