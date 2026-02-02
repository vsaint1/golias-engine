

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_texcoord;
layout(location = 3) in vec3 a_normal;
layout(location = 4) in vec4 a_bone_indices;
layout(location = 5) in vec4 a_bone_weights;

out vec3 v_color;
out vec2 v_texcoord;
out vec3 v_normal;
out vec3 v_frag_pos;
out float v_view_depth;
out vec4 v_frag_pos_light_space_cascade0;
out vec4 v_frag_pos_light_space_cascade1;
out vec4 v_frag_pos_light_space_cascade2;
out vec4 v_frag_pos_light_space_cascade3;

uniform mat4 MODEL_MATRIX;
uniform mat4 VIEW_MATRIX;
uniform mat4 PROJECTION_MATRIX;

uniform mat4 LIGHT_SPACE_MATRIX_CASCADE_0;
uniform mat4 LIGHT_SPACE_MATRIX_CASCADE_1;
uniform mat4 LIGHT_SPACE_MATRIX_CASCADE_2;
uniform mat4 LIGHT_SPACE_MATRIX_CASCADE_3;

uniform int USE_SKINNING;
const int MAX_BONES = 128;
uniform mat4 BONE_MATRICES[MAX_BONES];

void main() {
    v_color = a_color;
    v_texcoord = a_texcoord;
    
    vec4 localPos = vec4(a_pos, 1.0);
    vec3 localNormal = a_normal;
    
    
    if (USE_SKINNING > 0) {
        mat4 boneTransform = mat4(0.0);
        
        for (int i = 0; i < 4; i++) {
            int boneIndex = int(a_bone_indices[i]);
            float boneWeight = a_bone_weights[i];
            
            if (boneWeight > 0.0 && boneIndex >= 0 && boneIndex < MAX_BONES) {
                boneTransform += BONE_MATRICES[boneIndex] * boneWeight;
            }
        }
        
        
        if (dot(a_bone_weights, vec4(1.0)) > 0.01) {
            localPos = boneTransform * localPos;
            localNormal = mat3(boneTransform) * localNormal;
        }
    }
    
    vec4 worldPos = MODEL_MATRIX * localPos;
    v_frag_pos = worldPos.xyz;
    
    vec4 viewPos = VIEW_MATRIX * worldPos;
    v_view_depth = -viewPos.z;
    
    v_frag_pos_light_space_cascade0 = LIGHT_SPACE_MATRIX_CASCADE_0 * worldPos;
    v_frag_pos_light_space_cascade1 = LIGHT_SPACE_MATRIX_CASCADE_1 * worldPos;
    v_frag_pos_light_space_cascade2 = LIGHT_SPACE_MATRIX_CASCADE_2 * worldPos;
    v_frag_pos_light_space_cascade3 = LIGHT_SPACE_MATRIX_CASCADE_3 * worldPos;

  
    
    if (length(localNormal) > 0.01) {
        mat3 normalMatrix = transpose(inverse(mat3(MODEL_MATRIX)));
        v_normal = normalize(normalMatrix * localNormal);
    } else {
        v_normal = vec3(0.0, 1.0, 0.0);
    }
    
    gl_Position = PROJECTION_MATRIX * VIEW_MATRIX * worldPos;
}