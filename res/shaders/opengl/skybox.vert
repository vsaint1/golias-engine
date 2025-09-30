layout(location = 0) in vec3 a_pos;

out vec3 UV;

uniform mat4 VIEW;
uniform mat4 PROJECTION;

void main() {
    UV = a_pos;
    vec4 pos = PROJECTION * mat4(mat3(VIEW)) * vec4(a_pos, 1.0);
    gl_Position = pos.xyww; 
}