
layout(location = 0) in vec3 a_pos;
layout(location = 4) in vec4 a_bone_indices;
layout(location = 5) in vec4 a_bone_weights;

uniform mat4 MODEL_MATRIX;
uniform mat4 LIGHT_SPACE_MATRIX;

uniform int USE_SKINNING;
const int MAX_BONES = 128;
uniform mat4 BONE_MATRICES[MAX_BONES];

void main() {
    vec4 localPos = vec4(a_pos, 1.0);
    
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
        }
    }
    
    gl_Position = LIGHT_SPACE_MATRIX * MODEL_MATRIX * localPos;
}
