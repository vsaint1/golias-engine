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
};

uniform Material material;
uniform bool USE_TEXTURE;



// --------------------------------------
// PCF shadow calculation (3x3 kernel)
// --------------------------------------
float calculate_shadow(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir)
{
    // Project from light-space to [0,1]
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    // Outside light frustum → no shadow
    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 ||
        proj_coords.y < 0.0 || proj_coords.y > 1.0)
        return 0.0;

    float current_depth = proj_coords.z;
    float NdotL = dot(normal, light_dir);
    if (NdotL < 0.0) return 0.0;

    // Bias to reduce acne
    float bias = max(0.0015 * (1.0 - NdotL), 0.0003);

    // --- Variable softness based on distance ---
    float distance_from_light = abs(frag_pos_light_space.z / frag_pos_light_space.w);
    float filter_radius = mix(0.0015, 0.005, clamp(distance_from_light * 0.5, 0.0, 1.0));

    vec2 texel_size = 1.0 / textureSize(SHADOW_TEXTURE, 0);

    // --- Poisson disk offsets for smoother sampling ---
    const vec2 poisson_disk[16] = vec2[](
        vec2(-0.942, -0.399), vec2( 0.945, -0.768),
        vec2(-0.094, -0.929), vec2( 0.345, -0.237),
        vec2(-0.802, -0.145), vec2(-0.421, -0.967),
        vec2(-0.99,  0.108), vec2(-0.445,  0.529),
        vec2(-0.24,  0.717), vec2(-0.122,  0.023),
        vec2( 0.287,  0.967), vec2( 0.548,  0.731),
        vec2( 0.753,  0.284), vec2( 0.964, -0.196),
        vec2( 0.414, -0.738), vec2( 0.605, -0.549)
    );

    // Random rotation to reduce banding
    float angle = fract(sin(dot(proj_coords.xy, vec2(12.9898, 78.233))) * 43758.5453) * 6.2831;
    mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));

    float shadow = 0.0;
    for (int i = 0; i < 16; ++i) {
        vec2 offset = rotation * poisson_disk[i] * filter_radius;
        float pcf_depth = texture(SHADOW_TEXTURE, proj_coords.xy + offset * texel_size).r;
        shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;
    }
    shadow /= 16.0;

    return shadow;
}


// DEBUG MODES:
// 0 = normal shading
// 1 = visualize normals (colored)
// 2 = visualize world position (colored)
// 3 = visualize UVs
// 4 = visualize albedo/texture only
// 5 = visualize light-space depth (shadow_map)
// 6 = visualize shadow intensity
// 7 = visualize NdotL (diffuse term)
// 8 = visualize specular highlights
// 9 = visualize view direction
int DEBUG_MODE = 0;


void main()
{
    // Compute normalized light-space depth once
    float depth_norm = clamp(FRAG_POS_LIGHT_SPACE.z / FRAG_POS_LIGHT_SPACE.w * 0.5 + 0.5, 0.0, 1.0);

    vec3 albedo = USE_TEXTURE ? texture(TEXTURE, UV).rgb : material.albedo;
    albedo *= INSTANCE_COLOR;

    vec3 N = normalize(NORMAL);
    vec3 V = normalize(CAMERA_POSITION - WORLD_POSITION);
    vec3 L = -normalize(LIGHT_DIRECTION);
    vec3 H = normalize(V + L);

    float shadow = calculate_shadow(FRAG_POS_LIGHT_SPACE, N, L);
    float NdotL = max(dot(N, L), 0.0);

    // =======================================
    // DEBUG MODES
    // =======================================
    if (DEBUG_MODE == 1) {
        // Colored normals: map [-1,1] to [0,1] RGB
        COLOR = vec4(N * 0.5 + 0.5, 1.0);
        return;
    }
    else if (DEBUG_MODE == 2) {
        // World position visualization (mod to keep colors visible)
        vec3 pos_color = fract(WORLD_POSITION * 0.1);
        COLOR = vec4(pos_color, 1.0);
        return;
    }
    else if (DEBUG_MODE == 3) {
        // UV coordinates (red = U, green = V)
        COLOR = vec4(UV, 0.0, 1.0);
        return;
    }
    else if (DEBUG_MODE == 4) {
        // Albedo/texture only (no lighting)
        COLOR = vec4(albedo, 1.0);
        return;
    }
    else if (DEBUG_MODE == 5) {
        // Light-space depth
        COLOR = vec4(vec3(depth_norm), 1.0);
        return;
    }
    else if (DEBUG_MODE == 6) {
        // Shadow intensity (black = lit, white = shadowed)
        COLOR = vec4(vec3(shadow), 1.0);
        return;
    }
    else if (DEBUG_MODE == 7) {
        // NdotL (diffuse lighting term)
        COLOR = vec4(vec3(NdotL), 1.0);
        return;
    }
    else if (DEBUG_MODE == 8) {
        // Specular highlights only
        float NdotH = max(dot(N, H), 0.0);
        float spec = pow(NdotH, 32.0);
        COLOR = vec4(vec3(spec), 1.0);
        return;
    }
    else if (DEBUG_MODE == 9) {
        // View direction (camera to fragment)
        COLOR = vec4(V * 0.5 + 0.5, 1.0);
        return;
    }

    // ---- NORMAL LIGHTING ----
    const float AMBIENT_INTENSITY = 0.05;
    vec3 ambient = AMBIENT_INTENSITY * LIGHT_COLOR * albedo;

    vec3 diffuse = NdotL * LIGHT_COLOR * albedo;

    float specular_strength = 0.5;
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 32.0);
    vec3 specular = specular_strength * spec * LIGHT_COLOR;

    vec3 color = ambient + (1.0 - shadow) * (diffuse + specular);
    color = pow(color, vec3(1.0 / 2.2)); // gamma correction

    COLOR = vec4(color, 1.0);
}
