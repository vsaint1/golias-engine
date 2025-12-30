

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;
layout(location = 3) in vec3 a_normal;

out vec3 v_color;
out vec2 v_texcoord;
out vec3 v_normal;
out vec3 v_frag_pos;

uniform mat4 MODEL_MATRIX;
uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;

void main() {
    v_color = a_color;
    v_texcoord = a_texcoord;
    
    vec4 worldPos = MODEL_MATRIX * vec4(a_pos, 1.0);
    v_frag_pos = worldPos.xyz;
    
    if (length(a_normal) > 0.01) {
        mat3 normalMatrix = transpose(inverse(mat3(MODEL_MATRIX)));
        v_normal = normalize(normalMatrix * a_normal);
    } else {
        v_normal = vec3(0.0, 1.0, 0.0);
    }
    
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * worldPos;
}