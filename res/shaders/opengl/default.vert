layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoord;

out vec3 POSITION;
out vec3 NORMAL;
out vec2 UV;
out vec4 LIGHT_SPACE_POSITION;

uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJECTION;
uniform mat4 LIGHT_MATRIX;

void main() {
    POSITION = vec3(MODEL * vec4(a_position, 1.0));
    NORMAL = mat3(transpose(inverse(MODEL))) * a_normal;
    UV = a_texCoord;
    LIGHT_SPACE_POSITION = LIGHT_MATRIX * vec4(POSITION, 1.0);
    gl_Position = PROJECTION * VIEW * vec4(POSITION, 1.0);
}