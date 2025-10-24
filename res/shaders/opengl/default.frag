out vec4 COLOR;

in vec3 NORMAL;
in vec3 WORLD_POSITION;
in vec2 UV;
in vec3 INSTANCE_COLOR;
in vec4 FRAG_POS_LIGHT_SPACE;

uniform sampler2D ALBEDO_TEXTURE;
uniform sampler2D NORMAL_MAP_TEXTURE;
uniform sampler2D METALLIC_ROUGHNESS_TEXTURE;
uniform sampler2D AO_TEXTURE; // AMBIENT OCCLUSION
uniform sampler2D EMISSION_TEXTURE;
uniform sampler2D SHADOW_TEXTURE;

uniform vec3 CAMERA_POSITION;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    vec3 emissive;
    float emissive_strength;
};

struct DIRECTIONAL_LIGHT {
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform Material material;
uniform bool USE_ALBEDO_TEXTURE;
uniform bool USE_NORMAL_MAP_TEXTURE;
uniform bool USE_METALLIC_ROUGHNESS_TEXTURE;
uniform bool USE_AO_TEXTURE;

uniform DIRECTIONAL_LIGHT directional_light;

const float PI = 3.14159265359;

// ============================================================================
// Shadow Mapping with PCF (Percentage Closer Filtering)
// ============================================================================
// References:
// - https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float calculate_shadow(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir)
{
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 ||
    proj_coords.y < 0.0 || proj_coords.y > 1.0)
        return 0.0;

    float current_depth = proj_coords.z;
    float n_dot_l = max(dot(normal, light_dir), 0.0);
    float bias = max(0.0005 * (1.0 - n_dot_l), 0.00005);

    vec2 texel_size = 1.0 / vec2(textureSize(SHADOW_TEXTURE, 0));
    const float radius = 1.f;

    float randAngle = fract(sin(dot(UV, vec2(12.9898, 78.233))) * 43758.5453);
    mat2 rot = mat2(cos(randAngle), -sin(randAngle), sin(randAngle), cos(randAngle));

    const vec2 poisson[16] = vec2[](
    vec2(-0.942, -0.399), vec2(0.945, -0.768), vec2(-0.094, -0.929), vec2(0.344, 0.293),
    vec2(-0.915, 0.457), vec2(-0.815, -0.879), vec2(-0.382, 0.276), vec2(0.974, 0.756),
    vec2(0.443, -0.975), vec2(0.537, -0.473), vec2(-0.264, -0.418), vec2(0.791, 0.190),
    vec2(-0.241, 0.997), vec2(0.199, 0.786), vec2(0.143, -0.141), vec2(0.276, 0.134)
    );

    float shadow = 0.0;
    for (int i = 0; i < 16; i++) {
        vec2 offset = rot * poisson[i] * texel_size * radius;
        float pcf_depth = texture(SHADOW_TEXTURE, proj_coords.xy + offset).r;
        shadow += (current_depth - bias) > pcf_depth ? 1.0 : 0.0;
    }

    shadow /= 16.0;
    return shadow;
}

// ============================================================================
// Normal Mapping (Tangent Space)
// ============================================================================
// References:
// - https://learnopengl.com/Advanced-Lighting/Normal-Mapping
// - http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
vec3 calculate_normal_map(vec2 uv, vec3 normal_ws) {
    vec3 tangent_normal = texture(NORMAL_MAP_TEXTURE, uv).rgb * 2.0 - 1.0;
    vec3 Q1 = dFdx(WORLD_POSITION);
    vec3 Q2 = dFdy(WORLD_POSITION);
    vec2 uv0 = dFdx(UV);
    vec2 uv1 = dFdy(UV);

    vec3 N = normalize(normal_ws);
    vec3 T = normalize(Q1 * uv1.t - Q2 * uv0.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangent_normal);
}

// ============================================================================
// PBR: Cook-Torrance Microfacet BRDF
// ============================================================================
// The Cook-Torrance specular BRDF consists of three components:
// - D: Normal Distribution Function (NDF) - describes microfacet orientation
// - G: Geometry Function - describes self-shadowing/masking of microfacets
// - F: Fresnel Equation - describes light reflection at different angles
//
// References:
// - https://learnopengl.com/PBR/Theory
// - "Real Shading in Unreal Engine 4" by Brian Karis (Epic Games)

// ============================================================================
// NDF: Trowbridge-Reitz GGX Distribution
// ============================================================================
float distribution_ggx(vec3 n, vec3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float n_dot_h = max(dot(n, h), 0.0);
    float n_dot_h2 = n_dot_h * n_dot_h;

    float denom = (n_dot_h2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0000001);
}

// ============================================================================
// Geometry: Schlick-GGX (for direct lighting)
// ============================================================================
// Describes microfacet self-shadowing (when some microfacets occlude others).
// Uses k = (roughness + 1)² / 8 for direct lighting (IBL uses different k).
float geometry_schlick_ggx(float n_dot_v, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = n_dot_v * (1.0 - k) + k;

    return n_dot_v / max(denom, 0.0000001);
}

// ============================================================================
// Geometry: Smith's Method
// ============================================================================
float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float n_dot_v = max(dot(n, v), 0.0);
    float n_dot_l = max(dot(n, l), 0.0);
    float ggx_v = geometry_schlick_ggx(n_dot_v, roughness);
    float ggx_l = geometry_schlick_ggx(n_dot_l, roughness);

    return ggx_v * ggx_l;
}

// ============================================================================
// Fresnel: Schlick's Approximation
// ============================================================================
vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// ============================================================================
// Fresnel: Schlick with Roughness (for ambient/IBL)
// ============================================================================
vec3 fresnel_schlick_roughness(float cos_theta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// ============================================================================
// PBR: Directional Light Contribution
// ============================================================================
// Calculates lighting contribution from a directional light using the
// Cook-Torrance microfacet BRDF model.
//
// The BRDF has two terms:
// 1. Diffuse (Lambertian): kD * albedo / π
// 2. Specular (Cook-Torrance): DFG / (4 * (N·V) * (N·L))
//
// Where:
// - kD is the diffuse coefficient (1 - kS) * (1 - metallic)
// - kS is the specular coefficient (equal to Fresnel F)
// - D is the normal distribution (GGX)
// - F is the Fresnel term (Schlick approximation)
// - G is the geometry term (Smith's method)
vec3 calculate_pbr_directional_light(DIRECTIONAL_LIGHT light, vec3 n, vec3 v, vec3 albedo, float metallic, float roughness, float shadow)
{
    vec3 l = normalize(-light.direction);
    vec3 h = normalize(v + l);

    float n_dot_l = max(dot(n, l), 0.0);
    float n_dot_v = max(dot(n, v), 0.0001);
    float h_dot_v = max(dot(h, v), 0.0);

    // Calculate base reflectivity (F0)
    // Dielectrics: ~0.04, Metals: use albedo color
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);

    // Cook-Torrance BRDF components
    float d = distribution_ggx(n, h, roughness);
    float g = geometry_smith(n, v, l, roughness);
    vec3 f = fresnel_schlick(h_dot_v, f0);

    // Specular BRDF: DFG / (4 * (N·V) * (N·L))
    vec3 numerator = d * g * f;
    float denominator = 4.0 * n_dot_v * n_dot_l + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy conservation: kS + kD = 1
    vec3 k_s = f; // Fresnel represents specular reflection
    vec3 k_d = vec3(1.0) - k_s;
    k_d *= 1.0 - metallic; // Metals have no diffuse reflection

    // Radiance from light source
    vec3 radiance = light.color * light.intensity;

    // Lambertian diffuse: albedo / π
    vec3 diffuse = k_d * albedo / PI;

    // Final outgoing radiance: (diffuse + specular) * radiance * (N·L) * shadow
    return (diffuse + specular) * radiance * n_dot_l * (1.0 - shadow);
}

// ============================================================================
// Debug Visualization Modes
// ============================================================================
int DEBUG_MODE = 0; // 0 -> Normal Rendering

void enable_debug_mode(int mode, vec3 n, vec3 l, vec2 uv, vec3 albedo, float shadow, float n_dot_l, float metallic, float roughness, float ao) {
    switch (mode) {
        case 1:
            COLOR = vec4(n * 0.5 + 0.5, 1.0); // Normals
            break;
        case 2:
            COLOR = vec4(uv, 0.0, 1.0); // UVs
            break;
        case 3:
            COLOR = vec4(albedo, 1.0); // Albedo
            break;
        case 4:
            COLOR = vec4(vec3(1.0 - shadow), 1.0); // Shadow
            break;
        case 5:
            COLOR = vec4(vec3(n_dot_l), 1.0); // Lighting
            break;
        case 6:
            COLOR = vec4(vec3(metallic), 1.0); // Metallic
            break;
        case 7:
            COLOR = vec4(vec3(roughness), 1.0); // Roughness
            break;
        case 8:
            COLOR = vec4(vec3(texture(AO_TEXTURE, UV).r), 1.0); // AO visualization
            break;
        default:
            COLOR = vec4(1.0, 0.0, 1.0, 1.0); // Invalid mode
            return;
    }

}

void main()
{

    vec3 albedo;
    float alpha = 1.0;

    if (USE_ALBEDO_TEXTURE) {
        vec4 tex_sample = texture(ALBEDO_TEXTURE, UV);
        if (tex_sample.a < 0.1)
        discard;
        // Convert sRGB to linear space for correct lighting calculations
        albedo = pow(tex_sample.rgb, vec3(2.2));
        alpha = tex_sample.a;
    } else {
        albedo = material.albedo;
    }

    albedo *= INSTANCE_COLOR;

    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = 0.0f;

    // Red = Ambient Occlusion, Green channel = roughness, Blue channel = metallic (glTF 2.0 format)
    if (USE_METALLIC_ROUGHNESS_TEXTURE) {
        vec3 mr_sample = texture(METALLIC_ROUGHNESS_TEXTURE, UV).rgb;
        metallic = mr_sample.b;
        roughness = mr_sample.g;
        ao = mr_sample.r;
    }

    // Ambient Occlusion Texture overrides R channel of Metallic-Roughness texture
    if (USE_AO_TEXTURE) {
        ao = texture(AO_TEXTURE, UV).r;
    }


    metallic = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.04, 1.0);
    ao = clamp(ao, 0.0, 1.0);

    // ========================================================================
    // Calculate Surface Normal
    // ========================================================================
    vec3 normal_ws = normalize(NORMAL);
    vec3 n = USE_NORMAL_MAP_TEXTURE ? calculate_normal_map(UV, normal_ws) : normal_ws;

    vec3 v = normalize(CAMERA_POSITION - WORLD_POSITION);
    vec3 l = normalize(-directional_light.direction);

    // ========================================================================
    // Shadow Calculation
    // ========================================================================
    float shadow = calculate_shadow(FRAG_POS_LIGHT_SPACE, normal_ws, l);

    // ========================================================================
    // Debug Modes
    // ========================================================================
    float n_dot_l = max(dot(n, l), 0.0);
    if (DEBUG_MODE > 0) {
        enable_debug_mode(DEBUG_MODE, n, l, UV, albedo, shadow, n_dot_l, metallic, roughness, ao);
        return;
    }

    // ========================================================================
    // Direct Lighting (PBR)
    // ========================================================================
    vec3 Lo = calculate_pbr_directional_light(
        directional_light,
        n, v,
        albedo,
        metallic,
        roughness,
        shadow
    );

    // ========================================================================
    // Ambient/Indirect Lighting (Simplified)
    // ========================================================================
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);
    vec3 f = fresnel_schlick_roughness(max(dot(n, v), 0.0), f0, roughness);

    vec3 k_s = f;
    vec3 k_d = 1.0 - k_s;
    k_d *= 1.0 - metallic;


    vec3 ambient_sky = vec3(0.5, 0.6, 0.7) * 0.15;
    vec3 ambient_ground = vec3(0.3, 0.25, 0.2) * 0.05;

    float sky_factor = n.y * 0.5 + 0.5; // Map -1..1 to 0..1
    vec3 ambient_color = mix(ambient_ground, ambient_sky, sky_factor);

    vec3 ambient = k_d * albedo * ambient_color * mix(1.0, ao, 0.5);

    vec3 color = ambient + Lo;

    // ========================================================================
    // Tone Mapping (ACES Filmic)
    // ========================================================================
    // ACES tone mapping provides better color preservation than Reinhard.
    // Reference: Stephen Hill, "Aces Filmic Tone Mapping Curve"
    // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    color = color * (2.51 * color + 0.03) / (color * (2.43 * color + 0.59) + 0.14);

    // ========================================================================
    // Gamma Correction (Linear to sRGB)
    // ========================================================================
    color = pow(color, vec3(1.0 / 2.2));

    COLOR = vec4(color, alpha);
}