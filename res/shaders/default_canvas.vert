layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texcoord;

out vec4 v_color;
out vec2 v_uv;

uniform mat4 PROJECTION_MATRIX;


void main(){
    v_uv = a_texcoord;
    v_color = a_color;
    gl_Position = PROJECTION_MATRIX *  vec4(a_pos, 1.0);

}