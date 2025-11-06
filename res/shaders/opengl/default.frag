out vec4 COLOR;

in vec3 POSITION;
in vec3 NORMAL;
in vec2 UV;
in vec4 LIGHT_SPACE_POSITION;

uniform vec3 CAMERA_POSITION_WORLD;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    bool cast_shadows;
};

uniform DirectionalLight dirLights[100];
uniform int numDirLights;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float inner_cut_off;
    float outer_cut_off;
};

uniform SpotLight spotLights[100];
uniform int numSpotLights;


struct Material {
    vec3 albedo;
    vec3 specular;
    float metallic;
    float roughness;
    float ao;
    vec3 emissive;
    float emissiveStrength;
    float ior; // index of refraction (e.g. glass ~1.5, water ~1.33)
};

uniform Material material;

// Texture samplers
uniform sampler2D ALBEDO_MAP;
uniform sampler2D SPECULAR_MAP;
uniform sampler2D METALLIC_MAP;
uniform sampler2D ROUGHNESS_MAP;
uniform sampler2D NORMAL_MAP;
uniform sampler2D AO_MAP;
uniform sampler2D EMISSIVE_MAP;
uniform sampler2D SHADOW_MAP;
uniform samplerCube ENVIRONMENT_MAP; // environment cubemap for reflection/refraction/IBL

uniform int has_features;

#define HAS_ALBEDO_MAP    (1 << 0)
#define HAS_SPECULAR_MAP  (1 << 1)
#define HAS_METALLIC_MAP  (1 << 2)
#define HAS_ROUGHNESS_MAP (1 << 3)
#define HAS_NORMAL_MAP    (1 << 4)
#define HAS_AO_MAP        (1 << 5)
#define HAS_EMISSIVE_MAP  (1 << 6)
#define HAS_IBL           (1 << 7)

#define HAS(flag) ((has_features & flag) != 0)

const float PI = 3.14159265359;

uniform float TIME;

// ============================================================================
// Shadow Mapping with PCF (Percentage Closer Filtering)
// ============================================================================
// References:
// - https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadow_calculation(vec4 frag_pos_light_space, vec3 N, vec3 L)
{
    vec3 projCoords = frag_pos_light_space.xyz / frag_pos_light_space.w;

    // Transform from [-1,1] to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.0005 * (1.0 - dot(N, L)), 0.0001);

    // PCF sample (3x3 kernel)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(SHADOW_MAP, 0));

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            float pcfDepth = texture(SHADOW_MAP, projCoords.xy + offset).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;


    shadow *= smoothstep(0.0, 1.0, projCoords.z);

    return shadow;
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
float distribution_ggx(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0000001);
}

// ============================================================================
// Geometry: Schlick-GGX (for direct lighting)
// ============================================================================
// Describes microfacet self-shadowing (when some microfacets occlude others).
// Uses k = (roughness + 1)² / 8 for direct lighting (IBL uses different k).
float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k + 0.0000001);
}

// -----------------------------------------------------------------------------
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);
    return ggx1 * ggx2;
}

// -----------------------------------------------------------------------------
// Fresnel - Schlick approximation
// -----------------------------------------------------------------------------
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Roughness-variant fresnel for more realistic grazing reflections
vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ============================================================================
// PBR Light Contribution Calculation
// ============================================================================
// Calculates the Cook-Torrance BRDF for a given light direction and radiance.
// Returns the combined diffuse and specular contribution.
vec3 calculate_pbr_contribution(
    vec3 N, // Surface normal
    vec3 V, // View direction
    vec3 L, // Light direction
    vec3 radiance, // Incoming light radiance
    vec3 F0, // Base reflectance
    vec3 albedo, // Surface albedo
    float metallic, // Metallic factor
    float roughness      // Roughness factor
)
{
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float NDF = distribution_ggx(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    vec3 F = fresnel_schlick(HdotV, F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// ============================================================================
// Normal Mapping (Tangent Space)
// ============================================================================
// References:
// - https://learnopengl.com/Advanced-Lighting/Normal-Mapping
// - http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
vec3 calculate_normal_map()
{
    vec3 tangentNormal = texture(NORMAL_MAP, UV).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(POSITION);
    vec3 Q2 = dFdy(POSITION);
    vec2 st1 = dFdx(UV);
    vec2 st2 = dFdy(UV);

    vec3 N = normalize(NORMAL);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


vec3 sample_reflection(vec3 I, vec3 N)
{
    vec3 R = reflect(I, N);
    return texture(ENVIRONMENT_MAP, R).rgb;
}

vec3 sample_refraction(vec3 I, vec3 N, float eta)
{
    vec3 R = refract(I, N, 1.0 / eta);
    return texture(ENVIRONMENT_MAP, R).rgb;
}

// ============================================================================
// Image-Based Lighting (IBL) using Environment Map
// ============================================================================
// Approximates IBL by sampling the environment map at different mip levels
// Lower mip levels = sharper reflections (low roughness)
// Higher mip levels = blurrier reflections (high roughness)
// References:
// - https://learnopengl.com/PBR/IBL/Diffuse-irradiance
// - https://learnopengl.com/PBR/IBL/Specular-IBL
vec3 calculate_ibl(
    vec3 N,
    vec3 V,
    vec3 F0,
    vec3 albedo,
    float metallic,
    float roughness,
    float ao
)
{
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);
    vec3 F = fresnel_schlick_roughness(NdotV, F0, roughness);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    const float DIFFUSE_MIP = 5.0;
    vec3 irradiance = textureLod(ENVIRONMENT_MAP, N, DIFFUSE_MIP).rgb;
    vec3 diffuse = kD * irradiance * albedo;

    const float MAX_REFLECTION_LOD = 7.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColor = textureLod(ENVIRONMENT_MAP, R, lod).rgb;

    vec2 envBRDF = vec2(1.0 - roughness, 1.0 - roughness);
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    return (diffuse + specular) * ao;
}


void main()
{
    vec4 albedoSample = texture(ALBEDO_MAP, UV);
    if (albedoSample.a < 0.1)
    discard;

    vec3 finalAlbedo = HAS(HAS_ALBEDO_MAP) ? pow(albedoSample.rgb, vec3(2.2)) : material.albedo;
    float finalMetallic = material.metallic;
    float finalRoughness = material.roughness;
    float finalAO = material.ao;

    // Red = Ambient Occlusion, Green channel = roughness, Blue channel = metallic (glTF 2.0 format)
    if (HAS(HAS_METALLIC_MAP)) {
        vec3 mr = texture(METALLIC_MAP, UV).rgb;
        finalAO = mr.r;
        finalRoughness = mr.g;
        finalMetallic = mr.b;
    }

//    vec3 finalSpecular = material.specular;
//    if (USE_SPECULAR_MAP) {
//        vec3 specSample = texture(SPECULAR_MAP, UV).rgb;
//        finalSpecular = pow(specSample, vec3(2.2)); // gamma-corrected
//    }

    if (HAS(HAS_AO_MAP))
    finalAO = texture(AO_MAP, UV).r;

    if (HAS(HAS_ROUGHNESS_MAP))
    finalRoughness = texture(ROUGHNESS_MAP, UV).r;

    finalMetallic = clamp(finalMetallic, 0.0, 1.0);
    finalRoughness = clamp(finalRoughness, 0.04, 1.0);
    finalAO = clamp(finalAO, 0.0, 1.0);

    vec3 finalEmissive = material.emissive * material.emissiveStrength;
    if (HAS(HAS_EMISSIVE_MAP)) {
        vec3 emissiveSample = texture(EMISSIVE_MAP, UV).rgb;
        finalEmissive = pow(emissiveSample, vec3(2.2)) * material.emissiveStrength;
    }

    // --- Normal & View Direction ---
    vec3 N = HAS(HAS_NORMAL_MAP) ? calculate_normal_map() : normalize(NORMAL);
    vec3 V = normalize(CAMERA_POSITION_WORLD - POSITION);
    vec3 I = normalize(POSITION - CAMERA_POSITION_WORLD); // incident ray for reflection/refraction

    // --- Base Reflectance ---
    vec3 F0 = mix(vec3(0.04), finalAlbedo, finalMetallic);
//    F0 = mix(F0, finalSpecular, 0.5); // blend specular color for non-metals
    // TODO: IOR

    // --- Lighting ---
    vec3 Lo = vec3(0.0);

    // Directional Lights
    for (int i = 0; i < numDirLights; ++i) {
        vec3 L = normalize(-dirLights[i].direction);
        vec3 radiance = dirLights[i].color;

        // Calculate PBR contribution
        vec3 contribution = calculate_pbr_contribution(
            N, V, L, radiance, F0, finalAlbedo, finalMetallic, finalRoughness
        );

        float shadow = 1.0;
        if (dirLights[i].cast_shadows) {
            shadow = 1.0 - shadow_calculation(LIGHT_SPACE_POSITION, N, L);
        }

        Lo += contribution * shadow;
    }

    // Spot Lights
    for (int i = 0; i < numSpotLights; ++i) {
        vec3 L = normalize(spotLights[i].position - POSITION);
        float dist = length(spotLights[i].position - POSITION);
        float attenuation = 1.0 / (dist * dist);

        // Spotlight cone attenuation
        float theta = dot(L, normalize(-spotLights[i].direction));
        float epsilon = spotLights[i].inner_cut_off - spotLights[i].outer_cut_off;
        float intensity = clamp((theta - spotLights[i].outer_cut_off) / epsilon, 0.0, 1.0);

        vec3 radiance = spotLights[i].color * attenuation * intensity;

        // Calculate PBR contribution
        vec3 contribution = calculate_pbr_contribution(
            N, V, L, radiance, F0, finalAlbedo, finalMetallic, finalRoughness
        );

        Lo += contribution;
    }

    // --- Image-Based Lighting (IBL) ---
    vec3 ibl_contribution = vec3(0.0);
    vec3 env_color = vec3(0.0);

    if (HAS(HAS_IBL)) {
        ibl_contribution = calculate_ibl(N, V, F0, finalAlbedo, finalMetallic, finalRoughness, finalAO);
    }

    // --- Ambient Light & Emission ---
    vec3 ambient = HAS(HAS_IBL) ? vec3(0.0) : vec3(0.03) * finalAlbedo * finalAO;
    vec3 color = ambient + Lo + ibl_contribution + env_color + finalEmissive;

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

    const float ALPHA = albedoSample.a;
    
    COLOR = vec4(color, ALPHA);
}