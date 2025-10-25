layout (location = 0) in vec3 a_position;

uniform mat4 LIGHT_MATRIX;
uniform mat4 MODEL;

void main() {
    gl_Position = LIGHT_MATRIX * MODEL * vec4(a_position, 1.0);
}