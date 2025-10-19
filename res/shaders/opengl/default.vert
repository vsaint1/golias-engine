
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

// 3,4,5,6 (mat4 = 4 vec4 attributes)
layout (location = 3) in mat4 a_instance_model; // per-instance model matrix

layout(location = 7) in vec3 a_instance_color; // per-instance color

// Bone data (for skeletal animation)
layout(location = 8) in ivec4 a_bone_ids;
layout(location = 9) in vec4 a_bone_weights;

out vec3 NORMAL;
out vec3 WORLD_POSITION;
out vec2 UV;
out vec3 INSTANCE_COLOR;
out vec4 FRAG_POS_LIGHT_SPACE;

uniform mat4 VIEW;
uniform mat4 PROJECTION;
uniform mat4 LIGHT_PROJECTION;

const int MAX_BONES = 250; // ~16KB limit

uniform bool USE_SKELETON;
uniform mat4 BONES[MAX_BONES];

void main() {
    vec3 pos = a_pos;
    vec3 norm = a_normal;
    
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
        
        pos = vec3(boneTransform * vec4(a_pos, 1.0));
        norm = mat3(boneTransform) * a_normal;
    }
    
    WORLD_POSITION = vec3(a_instance_model * vec4(pos, 1.0));
    NORMAL = mat3(transpose(inverse(a_instance_model))) * norm;
    UV = a_tex_coord;
    gl_Position = PROJECTION * VIEW * vec4(WORLD_POSITION, 1.0);
    INSTANCE_COLOR = a_instance_color;
    FRAG_POS_LIGHT_SPACE = LIGHT_PROJECTION * vec4(WORLD_POSITION, 1.0);
}