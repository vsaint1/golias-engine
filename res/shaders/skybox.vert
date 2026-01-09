
layout(location = 0) in vec3 a_pos;

out vec3 v_texcoord;

uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;

void main() {
    v_texcoord = a_pos;
    
    mat4 viewNoTranslation = mat4(mat3(VIEW_MATRIX));
    
    vec4 pos = PROJECTION_MATRIX * viewNoTranslation * vec4(a_pos, 1.0);
    
    
    gl_Position = vec4(pos.x, pos.y, pos.w - 0.0001, pos.w);
}
