out vec4 COLOR;

in vec3 NORMAL; // vertex normal in world-space
in vec3 WORLD_POSITION;
in vec2 UV;
in vec3 INSTANCE_COLOR;
in vec4 FRAG_POS_LIGHT_SPACE;

uniform sampler2D TEXTURE;
uniform sampler2D NORMAL_TEXTURE;
uniform sampler2D SHADOW_TEXTURE;

// DIRECTIONAL LIGHT (SUN)
// NOTE: LIGHT_DIRECTION is the direction the light is *coming FROM*
uniform vec3 LIGHT_DIRECTION;
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
uniform bool USE_NORMAL_MAP;


float calculate_shadow(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir)
{
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 ||
    proj_coords.y < 0.0 || proj_coords.y > 1.0)
    return 0.0;

    float current_depth = proj_coords.z;
    float NdotL = max(dot(normal, light_dir), 0.0);

    if (NdotL < 0.01) return 1.0;

    float bias = 0.0005 + 0.001 * (1.0 - NdotL);

    vec2 texel_size = 1.0 / vec2(textureSize(SHADOW_TEXTURE, 0));

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

    float distance_to_camera = length(CAMERA_POSITION - WORLD_POSITION);
    float adaptive_radius = mix(0.5, 2.5, clamp(distance_to_camera / 50.0, 0.0, 1.0));
    float depth_scale = mix(1.0, 0.3, current_depth);
    adaptive_radius *= depth_scale;

    float shadow = 0.0;
    int sample_count = distance_to_camera < 20.0 ? 32 : 12;

    for (int i = 0; i < sample_count; i++)
    {
        vec2 offset = samples[i] * texel_size * adaptive_radius;
        float pcf_depth = texture(SHADOW_TEXTURE, proj_coords.xy + offset).r;
        shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;
    }
    shadow /= float(sample_count);
    shadow = smoothstep(0.0, 1.0, shadow);

    return shadow;
}


vec3 calculate_normal_map(vec2 uv, vec3 normal_ws){

    vec3 tangent_normal = texture(NORMAL_TEXTURE, UV).rgb * 2.0 - 1.0; // Convert from [0,1] to [-1,1]

    // R X-AXIS
    // G Y-AXIS
    // B Z-AXIS
    vec3 N = normalize(normal_ws);

    vec3 Q1  = dFdx(WORLD_POSITION);
    vec3 Q2  = dFdy(WORLD_POSITION);
    vec2 uv0 = dFdx(UV);
    vec2 uv1 = dFdy(UV);

    vec3 T  = normalize(Q1 * uv0.t - Q2 * uv1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    N = normalize(TBN* tangent_normal);

    return N;
}


// DEBUG MODES:
// 0 = normal shading (DEFAULT)
// 1 = visualize normals (colored)
// 2 = visualize UVs
// 3 = visualize albedo/texture only
// 4 = visualize shadow intensity
// 5 = visualize NdotL (diffuse term)
// 6 = visualize normal map (tangent space)
// 7 = visualize light direction
uniform int DEBUG_MODE;

void enable_debug_mode(int mode, vec3 N, vec3 L, vec2 uv, vec3 albedo, float shadow, float NdotL){
    if (mode == 1) {
        // Visualize normals (world space)
        COLOR = vec4(N * 0.5 + 0.5, 1.0);
    } else if (mode == 2) {
        // Visualize UVs
        COLOR = vec4(uv, 0.0, 1.0);
    } else if (mode == 3) {
        // Visualize albedo only
        COLOR = vec4(albedo, 1.0);
    } else if (mode == 4) {
        // Visualize shadow intensity (white = lit, black = shadowed)
        float shadow_vis = 1.0 - shadow;
        COLOR = vec4(vec3(shadow_vis), 1.0);
    } else if (mode == 5) {
        // Visualize NdotL (diffuse term)
        COLOR = vec4(vec3(NdotL), 1.0);
    } else if (mode == 6) {
        // Visualize normal map (tangent space, raw)
        if (USE_NORMAL_MAP) {
            vec3 tangent_normal = texture(NORMAL_TEXTURE, uv).rgb;
            COLOR = vec4(tangent_normal, 1.0);
        } else {
            COLOR = vec4(0.5, 0.5, 1.0, 1.0); // Default tangent space normal
        }
    } else if (mode == 7) {
        // Visualize light direction
        COLOR = vec4(L * 0.5 + 0.5, 1.0);
    }
}

void main()
{
    vec3 normal_ws = normalize(NORMAL);

    vec3 N = normalize(NORMAL);

    if (USE_NORMAL_MAP) {
        N = calculate_normal_map(UV, normal_ws);
    }

    vec3 albedo;
    float alpha = 1.0;

    if (USE_TEXTURE) {
        vec4 tex_sample = texture(TEXTURE, UV);
        if (tex_sample.a < 0.1)
            discard;
        albedo = tex_sample.rgb;
        alpha = tex_sample.a;
    } else {
        albedo = material.albedo;
    }

    albedo *= INSTANCE_COLOR;

    // View direction (towards camera)
    vec3 V = normalize(CAMERA_POSITION - WORLD_POSITION);

    // Light direction (towards light source)
    vec3 L = -normalize(LIGHT_DIRECTION);

    // Half vector for Blinn-Phong
    vec3 H = normalize(V + L);

    float shadow = calculate_shadow(FRAG_POS_LIGHT_SPACE, normal_ws, L);

    float NdotL = max(dot(N, L), 0.0);

    if (DEBUG_MODE > 0) {
        enable_debug_mode(DEBUG_MODE, N, L, UV, albedo, shadow, NdotL);
        return;
    }

    // --- Blinn-Phong Lighting calculation ---
    const float AMBIENT_INTENSITY = 0.15;
    vec3 ambient = AMBIENT_INTENSITY * LIGHT_COLOR * albedo;

    // Diffuse (Lambertian)
    vec3 diffuse = NdotL * LIGHT_COLOR * albedo;

    // Specular (Blinn-Phong)
    float specular_strength = 0.5;
    float NdotH = max(dot(N, H), 0.0);
    float shininess = 32.0;
    float spec = pow(NdotH, shininess);
    vec3 specular = specular_strength * spec * LIGHT_COLOR;

    vec3 color = ambient + (1.0 - shadow) * (diffuse + specular);

    // Gamma correction (linear to sRGB)
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    COLOR = vec4(color, alpha);
}
