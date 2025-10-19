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

// PCF shadow calculation with 3x3 sampling for softer shadows
float calculate_shadow(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir) {
   
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    
    proj_coords = proj_coords * 0.5 + 0.5;
    
    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || 
        proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }
    
    float current_depth = proj_coords.z;
    
  
    float NdotL = dot(normal, light_dir);
    
    if (NdotL < 0.0) {
        return 0.0;
    }
    
    float bias = max(0.005 * (1.0 - NdotL), 0.0005);
    
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(SHADOW_TEXTURE, 0);
    
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texel_size;
            float pcf_depth = texture(SHADOW_TEXTURE, proj_coords.xy + offset).r;
            shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // Average of 9 samples
    
    return shadow;
}

// ------------------------
// Simple Blinn-Phong Lighting
// ------------------------
void main() {
    
 
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
    
    // DEBUG: Visualize shadow values directly (white = shadowed, black = lit)
    // COLOR = vec4(vec3(shadow), 1.0); return;
    
    // DEBUG: Visualize shadow values directly (0=lit, 1=shadowed)
    // COLOR = vec4(vec3(shadow), 1.0); return;
    
 
    const float AMBIENT_INTENSITY = 0.05; 
    vec3 ambient = AMBIENT_INTENSITY * LIGHT_COLOR * albedo;
    
    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * LIGHT_COLOR * albedo;
    
    // Specular (Blinn-Phong)
    float specular_strength = 0.5;
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 32.0); // Shininess = 32
    vec3 specular = specular_strength * spec * LIGHT_COLOR;
    
    // Combine: ambient + (diffuse + specular) * (1.0 - shadow)
    // Shadow reduces the lit portions, leaving only ambient in shadowed areas
    vec3 color = ambient + (1.0 - shadow) * (diffuse + specular);
    
    if (shadow > 0.1) {
        color = mix(color, vec3(0.0, 0.0, 0.0), 0.5); 
    }
    
    // Simple gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    COLOR = vec4(color, 1.0);
}