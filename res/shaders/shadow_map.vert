
layout(location = 0) in vec3 a_pos;

uniform mat4 MODEL_MATRIX;
uniform mat4 LIGHT_SPACE_MATRIX;

void main() {
    gl_Position = LIGHT_SPACE_MATRIX * MODEL_MATRIX * vec4(a_pos, 1.0);
}
