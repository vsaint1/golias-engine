layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 3) in mat4 a_instance_model; // per-instance model matrix

// Bone data (for skeletal animation)
layout(location = 8) in ivec4 a_bone_ids;
layout(location = 9) in vec4 a_bone_weights;

uniform mat4 LIGHT_PROJECTION;

const int MAX_BONES = 250; // ~16KB limit

uniform bool USE_SKELETON;
uniform mat4 BONES[MAX_BONES];

void main() {
    vec3 pos = a_position;
    
    if (USE_SKELETON) {
        mat4 boneTransform = mat4(1.0); 
        float bone_weights_sum = 0.0;
        
        for (int i = 0; i < 4; ++i) {
            float w = a_bone_weights[i];
            if (w == 0.0) continue; 
            
            int id = a_bone_ids[i];
            if (id >= 0 && id < MAX_BONES) {
               
                if (bone_weights_sum == 0.0) {
                    boneTransform = BONES[id] * w; 
                } else {
                    boneTransform += BONES[id] * w; 
                }

                bone_weights_sum += w;
            }
        }
        
        pos = vec3(boneTransform * vec4(a_position, 1.0));
    }
    
    vec3 WORLD_POSITION = vec3(a_instance_model * vec4(pos, 1.0));
    gl_Position = LIGHT_PROJECTION * vec4(WORLD_POSITION, 1.0);
}