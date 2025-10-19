out vec4 COLOR;

in vec3 NORMAL;
in vec3 WORLD_POSITION;
in vec2 UV;
in vec3 INSTANCE_COLOR;
in vec4 FRAG_POS_LIGHT_SPACE;

uniform sampler2D TEXTURE;
uniform sampler2D SHADOW_TEXTURE;

// DIRECTIONAL LIGHT (SUN)
uniform vec3 LIGHT_DIRECTION; // Direction light is coming FROM
uniform vec3 LIGHT_COLOR; 

uniform vec3 CAMERA_POSITION; 

struct Metallic {
    vec3 specular;
    float value; 
};

struct Material {
    vec3 albedo;
    Metallic metallic;
    float roughness; 
    // TODO: handle textures later
};

uniform Material material;
uniform bool USE_TEXTURE;

// PCF Shadow calculation with soft edges - returns 0.0 (no shadow) to 1.0 (full shadow)
float calculate_shadow(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir) {
    // Perspective divide
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    
    // Transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    
    // Outside shadow map bounds = no shadow
    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || 
        proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }
    
    // Current fragment depth
    float current_depth = proj_coords.z;
    
    // Improved bias calculation based on surface angle to light
    // Surfaces perpendicular to light need less bias, surfaces at grazing angles need more
    float cos_theta = max(dot(normal, light_dir), 0.0);
    float bias = mix(0.01, 0.002, cos_theta); // Increased bias to reduce acne
    
    // PCF - sample shadow map multiple times for soft shadows
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(SHADOW_TEXTURE, 0);
    
    // 5x5 PCF kernel for softer shadows
    int samples = 0;
    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            vec2 offset = vec2(x, y) * texel_size;
            float pcf_depth = texture(SHADOW_TEXTURE, proj_coords.xy + offset).r;
            shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;
            samples++;
        }
    }
    shadow /= float(samples); // Average all samples
    
    // Fade out shadows at edges of shadow map
    float edge_fade = 1.0;
    vec2 edge_dist = abs(proj_coords.xy - 0.5);
    float max_edge = max(edge_dist.x, edge_dist.y);
    if (max_edge > 0.45) {
        edge_fade = 1.0 - smoothstep(0.45, 0.5, max_edge);
    }
    
    return shadow * edge_fade;
}

// ------------------------
// Simple Blinn-Phong Lighting
// ------------------------
void main() {
    
    // DEBUG: Visualize shadow map - UNCOMMENT TO DEBUG
    vec3 proj_coords = (FRAG_POS_LIGHT_SPACE.xyz / FRAG_POS_LIGHT_SPACE.w) * 0.5 + 0.5;
    float depth = texture(SHADOW_TEXTURE, proj_coords.xy).r;
    COLOR = vec4(vec3(depth), 1.0); return;
    
    // ------------------------
    // Base color
    // ------------------------
    vec3 albedo = USE_TEXTURE ? texture(TEXTURE, UV).rgb : material.albedo;
    albedo *= INSTANCE_COLOR;

    // ------------------------
    // Geometry
    // ------------------------
    vec3 N = normalize(NORMAL);
    vec3 V = normalize(CAMERA_POSITION - WORLD_POSITION);
    
    // Directional light: LIGHT_DIRECTION is direction light comes FROM
    // Negate to get direction TO the light (for lighting calculations)
    vec3 L = -normalize(LIGHT_DIRECTION);
    
    vec3 H = normalize(V + L); // Half vector

    // Calculate shadow first
    float shadow = calculate_shadow(FRAG_POS_LIGHT_SPACE, N, L);
    
    // ------------------------
    // Simple Blinn-Phong Lighting
    // ------------------------
    
    // Ambient
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * LIGHT_COLOR * albedo;
    
    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * LIGHT_COLOR * albedo;
    
    // Specular (Blinn-Phong)
    float specular_strength = 0.5;
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 32.0); // Shininess = 32
    vec3 specular = specular_strength * spec * LIGHT_COLOR;
    
    // Combine: ambient + (diffuse + specular) * shadow
    vec3 color = ambient + (diffuse + specular) * (1.0 - shadow);
    
    // Simple gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    COLOR = vec4(color, 1.0);
}