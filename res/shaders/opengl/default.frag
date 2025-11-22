out vec4 COLOR;

#define MAX_LIGHTS 50

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

uniform DirectionalLight dirLights[MAX_LIGHTS];
uniform int numDirLights;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float inner_cut_off;
    float outer_cut_off;
};

uniform SpotLight spotLights[MAX_LIGHTS];
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

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0)
    return 0.0;

    float currentDepth = projCoords.z;

    float bias = max(0.001 * (1.0 - dot(N, L)), 0.0005);

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
// NDF: Trowbridge-Reitz GGX Distribution (Improved)
// ============================================================================
// Better energy conservation and numerical stability
float distribution_ggx(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001);
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
    return NdotV / (NdotV * (1.0 - k) + k + 0.0001);
}

// Smith Joint Masking-Shadowing Function (uncorrelated)
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);
    return ggx1 * ggx2;
}

// Height-Correlated Smith G (more accurate, recommended)
float geometry_smith_correlated(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0001);
    float NdotL = max(dot(N, L), 0.0001);

    float a = roughness * roughness;
    float a2 = a * a;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);

    return 0.5 / max(GGXV + GGXL, 0.0001);
}

// ============================================================================
// Fresnel - Schlick approximation
// ============================================================================
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Roughness-variant fresnel for more realistic grazing reflections
vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Diffuse Fresnel (Disney/Burley diffuse)
// Provides energy conservation for the diffuse term
float diffuse_fresnel(float NdotL, float NdotV, float LdotH, float roughness)
{
    float FD90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    float FdV = 1.0 + (FD90 - 1.0) * pow(1.0 - NdotV, 5.0);
    float FdL = 1.0 + (FD90 - 1.0) * pow(1.0 - NdotL, 5.0);
    return FdV * FdL;
}

// ============================================================================
// PBR Light Contribution Calculation (Improved)
// ============================================================================
// Calculates the Cook-Torrance BRDF for a given light direction and radiance.
// Returns the combined diffuse and specular contribution with better energy conservation.
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

    float NdotV = max(dot(N, V), 0.0001);
    float NdotL = max(dot(N, L), 0.0001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    // Cook-Torrance specular BRDF
    float D = distribution_ggx(N, H, roughness);
    float G = geometry_smith_correlated(N, V, L, roughness); // Use improved correlated version
    vec3 F = fresnel_schlick(HdotV, F0);

    // Specular contribution
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular = numerator / max(denominator, 0.0001);

    // Energy conservation: kS is specular, kD is diffuse
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // Metals have no diffuse

    // Improved diffuse with Disney/Burley model
    float diffuse_fresnel_factor = diffuse_fresnel(NdotL, NdotV, LdotH, roughness);
    vec3 diffuse = (kD * albedo / PI) * diffuse_fresnel_factor;

    // Combine diffuse and specular
    return (diffuse + specular) * radiance * NdotL;
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


// ============================================================================
// Environment Reflections and Refractions
// ============================================================================
// For highly reflective or refractive materials (glass, water, mirrors)

vec3 sample_reflection(vec3 I, vec3 N, float roughness)
{
    vec3 R = reflect(I, N);
    // Use roughness to determine blur level
    float lod = roughness * 8.0;
    return textureLod(ENVIRONMENT_MAP, R, lod).rgb;
}

vec3 sample_refraction(vec3 I, vec3 N, float eta, float roughness)
{
    vec3 R = refract(I, N, eta);
    // Slightly blur refracted environment based on roughness
    float lod = roughness * 4.0;

    // Handle total internal reflection
    if (dot(R, R) < 0.001) {
        R = reflect(I, N);
    }

    return textureLod(ENVIRONMENT_MAP, R, lod).rgb;
}

// Calculate Fresnel effect for dielectric materials (glass, water)
// Returns the ratio of reflection vs refraction
float fresnel_dielectric(vec3 I, vec3 N, float ior)
{
    float cosi = clamp(dot(I, N), -1.0, 1.0);
    float etai = 1.0;
    float etat = ior;

    if (cosi > 0.0) {
        float temp = etai;
        etai = etat;
        etat = temp;
    }

    float sint = etai / etat * sqrt(max(0.0, 1.0 - cosi * cosi));

    if (sint >= 1.0) {
        return 1.0; // Total internal reflection
    }

    float cost = sqrt(max(0.0, 1.0 - sint * sint));
    cosi = abs(cosi);

    // Fresnel equations for s and p polarized light
    float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
    float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));

    return (Rs * Rs + Rp * Rp) / 2.0;
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

    // Diffuse IBL (irradiance)
    const float DIFFUSE_MIP = 5.0;
    vec3 irradiance = textureLod(ENVIRONMENT_MAP, N, DIFFUSE_MIP).rgb;
    vec3 diffuse = kD * irradiance * albedo;

    // Specular IBL (prefiltered environment) - IMPROVED for sharper reflections
    const float MAX_REFLECTION_LOD = 6.0; // Reduced for sharper reflections
    float lod = roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColor = textureLod(ENVIRONMENT_MAP, R, lod).rgb;

    // Better environmental BRDF approximation
    vec2 envBRDF = vec2(1.0 - roughness, 1.0 - roughness);

    // Boost specular for metals
    float metallic_boost = mix(1.0, 2.0, metallic); // Metals get 2x stronger reflections
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y) * metallic_boost;

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
    vec3 env_reflection = vec3(0.0);
    vec3 env_refraction = vec3(0.0);

    if (HAS(HAS_IBL)) {
        // Standard IBL for opaque materials
        ibl_contribution = calculate_ibl(N, V, F0, finalAlbedo, finalMetallic, finalRoughness, finalAO) * 0.5; // Increased from 0.3

        // Enhanced reflections for metallic materials (SIGNIFICANTLY BOOSTED)
        if (finalMetallic > 0.5) {
            vec3 R = reflect(I, N);
            float lod = finalRoughness * 8.0;
            vec3 metallic_reflection = textureLod(ENVIRONMENT_MAP, R, lod).rgb;

            float NdotV = max(dot(N, V), 0.0);
            vec3 F = fresnel_schlick_roughness(NdotV, F0, finalRoughness);

            // MUCH stronger metallic reflections for chrome/mirror effect
            float metallic_strength = mix(1.5, 3.0, finalMetallic); // More metallic = stronger reflection
            float roughness_factor = 1.0 - finalRoughness * 0.3; // Less penalty for roughness

            env_reflection = metallic_reflection * F * roughness_factor * metallic_strength;
            ibl_contribution += env_reflection * finalMetallic;
        }

        // Refraction for transparent materials (when alpha < 1.0 and IOR > 1.0)
        if (albedoSample.a < 0.99 && material.ior > 1.0) {
            float eta = 1.0 / material.ior;

            // Calculate Fresnel for dielectric
            float fresnelAmount = fresnel_dielectric(I, N, material.ior);

            // Sample both reflection and refraction
            vec3 refl = sample_reflection(I, N, finalRoughness);
            vec3 refr = sample_refraction(I, N, eta, finalRoughness);

            // Blend based on Fresnel and roughness
            env_refraction = mix(refr, refl, fresnelAmount);

            // Apply to transparent parts
            float transparency = 1.0 - albedoSample.a;
            ibl_contribution = mix(ibl_contribution, env_refraction * 0.8, transparency * 0.8);
        }
    }

    // --- Ambient Light & Emission ---
    // When IBL is disabled, use improved hemisphere ambient
    vec3 ambient = vec3(0.0);
    if (!HAS(HAS_IBL)) {
        // Hemisphere ambient lighting approximation
        // Sky color (top) and ground color (bottom)
        vec3 sky_color = vec3(0.05, 0.05, 0.08); // Slightly blue
        vec3 ground_color = vec3(0.02, 0.02, 0.02); // Dark ground

        // Blend based on normal direction (up = sky, down = ground)
        float sky_factor = N.y * 0.5 + 0.5; // Remap -1..1 to 0..1
        vec3 ambient_color = mix(ground_color, sky_color, sky_factor);

        // Apply Fresnel for ambient (more realistic)
        float NdotV = max(dot(N, V), 0.0);
        vec3 F_ambient = fresnel_schlick_roughness(NdotV, F0, finalRoughness);
        vec3 kD_ambient = (vec3(1.0) - F_ambient) * (1.0 - finalMetallic);

        ambient = ambient_color * finalAlbedo * kD_ambient * finalAO;
    }

    vec3 color = ambient + Lo + ibl_contribution + finalEmissive;

    // ========================================================================
    // Exposure Control
    // ========================================================================
    // Automatic exposure based on scene brightness
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float auto_exposure = 1.0 / (1.0 + luma * 0.5); // Adaptive
    float manual_exposure = 0.8; // Manual control
    float exposure = mix(manual_exposure, auto_exposure, 0.3); // Blend
    color *= exposure;

    // ========================================================================
    // Tone Mapping (Improved ACES)
    // ========================================================================
    // ACES tone mapping provides better color preservation than Reinhard.
    // Reference: Stephen Hill, "Aces Filmic Tone Mapping Curve"
    // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

    // Improved ACES with better midtones
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);

    // ========================================================================
    // Gamma Correction (Linear to sRGB)
    // ========================================================================
    color = pow(color, vec3(1.0 / 2.2));

    float ALPHA = albedoSample.a;

    COLOR = vec4(color, ALPHA);
}