out vec4 COLOR;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform vec3 camPos;

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

// Material properties
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform vec3 emissive;
uniform float emissiveStrength;
uniform float ior; // index of refraction (e.g. glass ~1.5, water ~1.33)

// Texture samplers
uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;
uniform sampler2D shadowMap;
uniform samplerCube envMap; // environment cubemap for reflection/refraction

// Usage flags
uniform bool useAlbedoMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useNormalMap;
uniform bool useAOMap;
uniform bool useEmissiveMap;
uniform bool useRefraction;
uniform bool useReflection;

const float PI = 3.14159265359;

// ============================================================================
// Shadow Mapping with PCF (Percentage Closer Filtering)
// ============================================================================
// References:
// - https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadow_calculation(vec4 frag_pos_light_space, vec3 N, vec3 L)
{
    vec3 projCoords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.002 * (1.0 - dot(N, L)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    if (projCoords.z > 1.0) shadow = 0.0;
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
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
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
    return NdotV / (NdotV * (1.0 - k) + k);
}

// -----------------------------------------------------------------------------
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometry_schlick_ggx(NdotV, roughness) * geometry_schlick_ggx(NdotL, roughness);
}

// -----------------------------------------------------------------------------
// Fresnel - Schlick approximation
// -----------------------------------------------------------------------------
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Roughness-variant fresnel for more realistic grazing reflections
vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ============================================================================
// Normal Mapping (Tangent Space)
// ============================================================================
// References:
// - https://learnopengl.com/Advanced-Lighting/Normal-Mapping
// - http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
vec3 calculate_normal_map()
{
    vec3 tangentNormal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


vec3 sample_reflection(vec3 I, vec3 N)
{
    vec3 R = reflect(I, N);
    return texture(envMap, R).rgb;
}

vec3 sample_refraction(vec3 I, vec3 N, float eta)
{
    vec3 R = refract(I, N, 1.0 / eta);
    return texture(envMap, R).rgb;
}


void main()
{
    if (texture(albedoMap, TexCoord).a < 0.1)
    discard;


    vec3 finalAlbedo = useAlbedoMap ? pow(texture(albedoMap, TexCoord).rgb, vec3(2.2)) : albedo;
    float finalMetallic = metallic;
    float finalRoughness = roughness;
    float finalAO = useAOMap ? texture(aoMap, TexCoord).r : ao;

    // Red = Ambient Occlusion, Green channel = roughness, Blue channel = metallic (glTF 2.0 format)
    if (useMetallicMap) {
        vec3 mr = texture(metallicMap, TexCoord).rgb;
        finalMetallic = mr.b;
        finalRoughness = mr.g;
    }

    if (useRoughnessMap)
    finalRoughness = texture(roughnessMap, TexCoord).r;

    vec3 finalEmissive = useEmissiveMap
    ? texture(emissiveMap, TexCoord).rgb * emissiveStrength
    : emissive * emissiveStrength;

    // --- Normal & View Direction ---
    vec3 N = useNormalMap ? calculate_normal_map() : normalize(Normal);
    vec3 V = normalize(camPos - FragPos);
    vec3 I = normalize(FragPos - camPos); // incident ray for reflection/refraction

    // --- Base Reflectance ---
    vec3 F0 = mix(vec3(0.04), finalAlbedo, finalMetallic);

    // --- Lighting ---
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numDirLights; ++i) {
        vec3 L = normalize(-dirLights[i].direction);
        vec3 H = normalize(V + L);
        vec3 radiance = dirLights[i].color;

        float NDF = distribution_ggx(N, H, finalRoughness);
        float G = geometry_smith(N, V, L, finalRoughness);
        vec3 F = fresnel_schlick_roughness(max(dot(H, V), 0.0), F0, finalRoughness);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - finalMetallic);
        vec3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = numerator / denom;

        float NdotL = max(dot(N, L), 0.0);
        float shadow = dirLights[i].cast_shadows ? (1.0 - shadow_calculation(FragPosLightSpace, N, L)) : 1.0;
        Lo += (kD * finalAlbedo / PI + specular) * radiance * NdotL * shadow;
    }


    for (int i = 0; i < numSpotLights; ++i) {
        vec3 L = normalize(spotLights[i].position - FragPos);
        vec3 H = normalize(V + L);
        float dist = length(spotLights[i].position - FragPos);
        float attenuation = 1.0 / (dist * dist);

        float theta = dot(L, normalize(-spotLights[i].direction));
        float epsilon = spotLights[i].inner_cut_off - spotLights[i].outer_cut_off;
        float intensity = clamp((theta - spotLights[i].outer_cut_off) / epsilon, 0.0, 1.0);

        vec3 radiance = spotLights[i].color * attenuation * intensity;

        float NDF = distribution_ggx(N, H, finalRoughness);
        float G = geometry_smith(N, V, L, finalRoughness);
        vec3 F = fresnel_schlick_roughness(max(dot(H, V), 0.0), F0, finalRoughness);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - finalMetallic);
        vec3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = numerator / denom;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * finalAlbedo / PI + specular) * radiance * NdotL;
    }

    // --- Environment Reflection & Refraction ---
    vec3 reflection_color = useReflection ? sample_reflection(I, N) : vec3(0.0);
    vec3 refraction_color = useRefraction ? sample_refraction(I, N, ior) : vec3(0.0);
    float fresnel_ratio = clamp(pow(1.0 - max(dot(N, V), 0.0), 5.0), 0.0, 1.0);

    vec3 env_color = mix(refraction_color, reflection_color, fresnel_ratio);

    // --- Ambient Light & Emission ---
    vec3 ambient = vec3(0.03) * finalAlbedo * finalAO;
    vec3 color = ambient + Lo + env_color + finalEmissive;

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

    COLOR = vec4(clamp(color, 0.0, 1.0), 1.0);
}
