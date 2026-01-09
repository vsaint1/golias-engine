in vec3 v_color;
in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_frag_pos;
in vec4 v_frag_pos_light_space;

out vec4 FRAG_COLOR;

/* ============================================================================
   Textures
   ============================================================================ */

uniform sampler2D ALBEDO_TEXTURE;
uniform sampler2D METALLIC_TEXTURE;
uniform sampler2D ROUGHNESS_TEXTURE;
uniform sampler2D NORMAL_TEXTURE;
uniform sampler2D AO_TEXTURE;
uniform sampler2D EMISSIVE_TEXTURE;

/* Shadow map generated from directional light */
uniform sampler2D SHADOW_MAP;

/* IBL textures */
uniform samplerCube IRRADIANCE_MAP;
uniform samplerCube PREFILTER_MAP;
uniform int USE_IBL;

/* ============================================================================
   Texture presence flags (bitmask)
   ============================================================================ */

const int HAS_ALBEDO_FLAG    = 1;
const int HAS_METALLIC_FLAG  = 2;
const int HAS_ROUGHNESS_FLAG = 4;
const int HAS_NORMAL_FLAG    = 8;
const int HAS_AO_FLAG        = 16;
const int HAS_EMISSIVE_FLAG  = 32;

uniform int TEXTURE_FLAGS;

const float PI = 3.14159265359;

/* ============================================================================
   Material representation after sampling
   ============================================================================ */

struct Material {
    vec4  albedo;     /* Base color (linear space) */
    float metallic;   /* 0 = dielectric, 1 = metal */
    float roughness;  /* Microfacet roughness */
    float ao;         /* Ambient occlusion */
    vec3  emissive;   /* Emissive contribution */
    vec3  normal;     /* Final shading normal */
};

/* Per-material uniform modifiers */
struct MaterialProperties {
    vec4  modulate;
    vec3  emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveStrength;
};

uniform MaterialProperties u_material;

/* ============================================================================
   Directional light
   ============================================================================ */

struct DirectionalLight {
    vec3  direction;
    vec3  color;
    float intensity;
    bool  castShadows;
};

const int MAX_DIRECTIONAL_LIGHTS = 32;
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directionalLightCount;

/* ============================================================================
   Scene uniforms
   ============================================================================ */

uniform vec3 u_viewPosition;
uniform float u_ambientStrength;

/* ============================================================================
   Cook–Torrance BRDF helpers
   ============================================================================ */

/* GGX / Trowbridge-Reitz normal distribution */
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

/* Schlick-GGX geometry term (visibility) */
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

/* Smith geometry term for both view and light */
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

/* Fresnel term using Schlick approximation */
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

/* ============================================================================
   Material sampling
   ============================================================================ */

Material SampleMaterial()
{
    Material mat;

    /* Albedo */
    mat.albedo =
        ((TEXTURE_FLAGS & HAS_ALBEDO_FLAG) != 0)
        ? texture(ALBEDO_TEXTURE, v_texcoord) * u_material.modulate
        : u_material.modulate;

    mat.albedo.rgb *= v_color;

    /* Metallic */
    mat.metallic =
        ((TEXTURE_FLAGS & HAS_METALLIC_FLAG) != 0)
        ? texture(METALLIC_TEXTURE, v_texcoord).b * u_material.metallicFactor
        : u_material.metallicFactor;

    /* Roughness */
    mat.roughness =
        ((TEXTURE_FLAGS & HAS_ROUGHNESS_FLAG) != 0)
        ? texture(ROUGHNESS_TEXTURE, v_texcoord).g * u_material.roughnessFactor
        : u_material.roughnessFactor;

    mat.roughness = max(mat.roughness, 0.04);

    /* Ambient occlusion */
    mat.ao =
        ((TEXTURE_FLAGS & HAS_AO_FLAG) != 0)
        ? texture(AO_TEXTURE, v_texcoord).r
        : 1.0;

    /* Emissive */
    mat.emissive =
        ((TEXTURE_FLAGS & HAS_EMISSIVE_FLAG) != 0)
        ? texture(EMISSIVE_TEXTURE, v_texcoord).rgb *
          u_material.emissiveFactor * u_material.emissiveStrength
        : vec3(0.0);

    /* Normal mapping (approximate, no TBN) */
    mat.normal = normalize(v_normal);
    if ((TEXTURE_FLAGS & HAS_NORMAL_FLAG) != 0) {
        vec3 n = texture(NORMAL_TEXTURE, v_texcoord).xyz * 2.0 - 1.0;
        mat.normal = normalize(mat.normal + n * 0.5);
    }

    return mat;
}

/* ============================================================================
   Directional shadow mapping with PCF
   ============================================================================ */

float CalculateShadow(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

    if (proj.x <= 0.0 || proj.x >= 1.0 ||
        proj.y <= 0.0 || proj.y >= 1.0)
        return 1.0;


    float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);
    vec2 texel = 1.0 / vec2(textureSize(SHADOW_MAP, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++) {
        float depth = texture(SHADOW_MAP, proj.xy + vec2(x, y) * texel).r;
        shadow += (proj.z - bias) > depth ? 0.0 : 1.0;
    }

    return shadow / 9.0;
}

/* ============================================================================
   IBL (Image-Based Lighting) Functions
   ============================================================================ */

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * 
            pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalculateIBL(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness, float ao)
{
    if (USE_IBL == 0) {
        return vec3(0.0);
    }

    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);
    
    vec3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    
    /* Diffuse irradiance */
    vec3 irradiance = texture(IRRADIANCE_MAP, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    /* Specular reflection with roughness-based mip selection */
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(PREFILTER_MAP, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec3 specular = prefilteredColor * F;
    
    return (kD * diffuse + specular) * ao;
}


void main()
{
    Material mat = SampleMaterial();

    vec3 N = mat.normal;
    vec3 V = normalize(u_viewPosition - v_frag_pos);

    /* Base reflectivity */
    vec3 F0 = mix(vec3(0.04), mat.albedo.rgb, mat.metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_directionalLightCount; i++) {
        DirectionalLight light = u_directionalLights[i];

        vec3 L = normalize(-light.direction);
        vec3 H = normalize(V + L);
        vec3 radiance = light.color * light.intensity;

        float shadow = 1.0;
        if (light.castShadows && i == 0)
            shadow = CalculateShadow(v_frag_pos_light_space, N, L);

        /* Cook–Torrance BRDF */
        float NDF = DistributionGGX(N, H, mat.roughness);
        float G   = GeometrySmith(N, V, L, mat.roughness);
        vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular =
            (NDF * G * F) /
            (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

        /* Lambertian diffuse */
        vec3 kD = (1.0 - F) * (1.0 - mat.metallic);
        vec3 diffuse = kD * mat.albedo.rgb / PI;

        Lo += (diffuse + specular) *
              radiance *
              max(dot(N, L), 0.0) *
              shadow;
    }

    /* Ambient term with optional IBL */
    vec3 ambient;
    if (USE_IBL == 1) {
        ambient = CalculateIBL(N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness, mat.ao);
    } else {
        ambient = u_ambientStrength * mat.albedo.rgb * mat.ao;
    }

    vec3 color = ambient + Lo + mat.emissive;

    // /* Tone mapping (Reinhard) */
    // color = color / (color + vec3(1.0));

    // /* Gamma correction */
    // color = pow(color, vec3(1.0 / 2.2));

    FRAG_COLOR = vec4(color, mat.albedo.a);
}
