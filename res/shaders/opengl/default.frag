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
// SHADOW CALCULATION BASED ON CAMERA DISTANCE
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
    float NdotL = max(dot(normal, light_dir), 0.0);
    
    if (NdotL < 0.01) return 1.0; // Surfaces facing away are in shadow

    float bias = 0.0005 + 0.001 * (1.0 - NdotL);
    

    vec2 texel_size = 1.0 / vec2(textureSize(SHADOW_TEXTURE, 0));   
     
    // Poisson disk sampling pattern - 32 samples for higher quality
    const vec2 samples[32] = vec2[](
        vec2(-0.94201624, -0.39906216),
        vec2(0.94558609, -0.76890725),
        vec2(-0.094184101, -0.92938870),
        vec2(0.34495938, 0.29387760),
        vec2(-0.91588581, 0.45771432),
        vec2(-0.81544232, -0.87912464),
        vec2(-0.38277543, 0.27676845),
        vec2(0.97484398, 0.75648379),
        vec2(0.44323325, -0.97511554),
        vec2(0.53742981, -0.47373420),
        vec2(-0.26496911, -0.41893023),
        vec2(0.79197514, 0.19090188),
        vec2(-0.24188840, 0.99706507),
        vec2(-0.81409955, 0.91437590),
        vec2(0.19984126, 0.78641367),
        vec2(0.14383161, -0.14100790),
        // Additional 16 samples for smoother shadows
        vec2(0.59490621, 0.44508049),
        vec2(-0.67888541, 0.06997144),
        vec2(0.18650039, -0.56448567),
        vec2(-0.12964488, 0.46467763),
        vec2(0.71628402, -0.31403765),
        vec2(-0.46671849, -0.69000623),
        vec2(0.32782465, 0.88611004),
        vec2(-0.58654654, 0.73139274),
        vec2(0.87689108, -0.01333499),
        vec2(-0.03935671, 0.85738134),
        vec2(0.06747698, -0.23559207),
        vec2(-0.39294919, -0.16617249),
        vec2(0.46717972, -0.70563352),
        vec2(-0.76943421, -0.45486257),
        vec2(0.27653325, 0.13415599),
        vec2(-0.52173114, 0.35023832)
    );
    
    // Distance-adaptive radius
    // Close up: smaller radius to avoid pixelation
    // Far away: larger radius for soft shadows
    float distance_to_camera = length(CAMERA_POSITION - WORLD_POSITION);
    float adaptive_radius = mix(0.5, 2.5, clamp(distance_to_camera / 50.0, 0.0, 1.0));
    
    // Also scale by depth to reduce perspective aliasing
    float depth_scale = mix(1.0, 0.3, current_depth);
    adaptive_radius *= depth_scale;
    
    float shadow = 0.0;
    
    // Sample count based on distance
    int sample_count = distance_to_camera < 20.0 ? 32 : 12;
    
    for(int i = 0; i < sample_count; i++)
    {
        vec2 offset = samples[i] * texel_size * adaptive_radius;
        float pcf_depth = texture(SHADOW_TEXTURE, proj_coords.xy + offset).r;
        shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;
    }
    shadow /= float(sample_count);
    
    // Smooth transition at shadow edges
    shadow = smoothstep(0.0, 1.0, shadow);

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
// TODO: use a uniform ???
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
