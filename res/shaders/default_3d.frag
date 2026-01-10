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

const int HAS_ALBEDO_FLAG = 1;
const int HAS_METALLIC_FLAG = 2;
const int HAS_ROUGHNESS_FLAG = 4;
const int HAS_NORMAL_FLAG = 8;
const int HAS_AO_FLAG = 16;
const int HAS_EMISSIVE_FLAG = 32;

uniform int TEXTURE_FLAGS;

const float PI = 3.14159265359;

/* ============================================================================
   Material representation after sampling
   ============================================================================ */

struct Material {
    vec4 albedo;     /* Base color (linear space) */
    float metallic;   /* 0 = dielectric, 1 = metal */
    float roughness;  /* Microfacet roughness */
    float ao;         /* Ambient occlusion */
    vec3 emissive;   /* Emissive contribution */
    vec3 normal;     /* Final shading normal */
};

/* Per-material uniform modifiers */
struct MaterialProperties {
    vec4 modulate;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveStrength;
};

uniform MaterialProperties u_material;

/* ============================================================================
   Directional light
   ============================================================================ */

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    bool castShadows;
};

const int MAX_DIRECTIONAL_LIGHTS = 32;
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directionalLightCount;

/* ============================================================================
   Point light
   ============================================================================ */

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float range;
    float constant;
    float linear;
    float quadratic;
};

const int MAX_POINT_LIGHTS = 32;
uniform PointLight u_pointLights[MAX_POINT_LIGHTS];
uniform int u_pointLightCount;

/* ============================================================================
   Spot light
   ============================================================================ */

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;
    float innerConeAngle;  // cos(angle) stored for efficiency
    float outerConeAngle;  // cos(angle) stored for efficiency
    float constant;
    float linear;
    float quadratic;
};

const int MAX_SPOT_LIGHTS = 32;
uniform SpotLight u_spotLights[MAX_SPOT_LIGHTS];
uniform int u_spotLightCount;

/* ============================================================================
   Scene uniforms
   ============================================================================ */

uniform vec3 u_viewPosition;
uniform float u_ambientStrength;

/* ============================================================================
   Cook–Torrance BRDF helpers
   ============================================================================ */

/* GGX / Trowbridge-Reitz normal distribution */
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

/* Schlick-GGX geometry term (visibility) */
float GeometrySchlickGGX(float NdotV, float roughness) {
    float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

/* Smith geometry term for both view and light */
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
        GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

/* Fresnel term using Schlick approximation */
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

/* ============================================================================
   Lighting contribution functions
   ============================================================================ */

/* Calculate contribution from a directional light */
vec3 CalculateDirectionalLight(
    DirectionalLight light,
    vec3 N,
    vec3 V,
    vec3 F0,
    vec3 albedo,
    float metallic,
    float roughness,
    float shadow
) {
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(V + L);
    vec3 radiance = light.color * light.intensity;

    /* Cook–Torrance BRDF */
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (NDF * G * F) /
        (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

    /* Lambertian diffuse */
    vec3 kD = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * max(dot(N, L), 0.0) * shadow;
}

/* Calculate contribution from a point light */
vec3 CalculatePointLight(
    PointLight light,
    vec3 fragPos,
    vec3 N,
    vec3 V,
    vec3 F0,
    vec3 albedo,
    float metallic,
    float roughness
) {
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    float distance = length(light.position - fragPos);

    /* Early exit if fragment is beyond light range */
    if(distance > light.range) {
        return vec3(0.0);
    }

    /* Calculate attenuation */
    float attenuation = 1.0 / (light.constant +
        light.linear * distance +
        light.quadratic * distance * distance);

    vec3 radiance = light.color * light.intensity * attenuation;

    /* Cook–Torrance BRDF */
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (NDF * G * F) /
        (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

    /* Lambertian diffuse */
    vec3 kD = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * max(dot(N, L), 0.0);
}

/* Calculate contribution from a spot light */
vec3 CalculateSpotLight(
    SpotLight light,
    vec3 fragPos,
    vec3 N,
    vec3 V,
    vec3 F0,
    vec3 albedo,
    float metallic,
    float roughness
) {
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    float distance = length(light.position - fragPos);

    /* Early exit if fragment is beyond light range */
    if(distance > light.range) {
        return vec3(0.0);
    }

    /* Calculate spotlight intensity based on cone angle */
    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.innerConeAngle - light.outerConeAngle;
    float spotIntensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);

    /* Early exit if fragment is outside spotlight cone */
    if(spotIntensity <= 0.0) {
        return vec3(0.0);
    }

    /* Calculate attenuation */
    float attenuation = 1.0 / (light.constant +
        light.linear * distance +
        light.quadratic * distance * distance);

    vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;

    /* Cook–Torrance BRDF */
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (NDF * G * F) /
        (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

    /* Lambertian diffuse */
    vec3 kD = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * max(dot(N, L), 0.0);
}

/* ============================================================================
   Material sampling
   ============================================================================ */

Material SampleMaterial() {
    Material mat;

    /* Albedo */
    mat.albedo = ((TEXTURE_FLAGS & HAS_ALBEDO_FLAG) != 0) ? texture(ALBEDO_TEXTURE, v_texcoord) * u_material.modulate : u_material.modulate;

    mat.albedo.rgb *= v_color;

    /* Metallic */
    mat.metallic = ((TEXTURE_FLAGS & HAS_METALLIC_FLAG) != 0) ? texture(METALLIC_TEXTURE, v_texcoord).b * u_material.metallicFactor : u_material.metallicFactor;

    /* Roughness */
    mat.roughness = ((TEXTURE_FLAGS & HAS_ROUGHNESS_FLAG) != 0) ? texture(ROUGHNESS_TEXTURE, v_texcoord).g * u_material.roughnessFactor : u_material.roughnessFactor;

    mat.roughness = max(mat.roughness, 0.04);

    /* Ambient occlusion */
    mat.ao = ((TEXTURE_FLAGS & HAS_AO_FLAG) != 0) ? texture(AO_TEXTURE, v_texcoord).r : 1.0;

    /* Emissive */
    mat.emissive = ((TEXTURE_FLAGS & HAS_EMISSIVE_FLAG) != 0) ? texture(EMISSIVE_TEXTURE, v_texcoord).rgb *
        u_material.emissiveFactor * u_material.emissiveStrength : vec3(0.0);

    /* Normal mapping (approximate, no TBN) */
    mat.normal = normalize(v_normal);
    if((TEXTURE_FLAGS & HAS_NORMAL_FLAG) != 0) {
        vec3 n = texture(NORMAL_TEXTURE, v_texcoord).xyz * 2.0 - 1.0;
        mat.normal = normalize(mat.normal + n * 0.5);
    }

    return mat;
}

/* ============================================================================
   Directional shadow mapping with PCF
   ============================================================================ */

float CalculateShadow(vec4 fragPosLightSpace, vec3 N, vec3 L) {
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

    if(proj.z > 1.0)
        return 1.0;

    if(proj.x <= 0.0 || proj.x >= 1.0 ||
        proj.y <= 0.0 || proj.y >= 1.0)
        return 1.0;

    float currentDepth = proj.z;
    float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);

    vec2 texelSize = 1.0 / vec2(textureSize(SHADOW_MAP, 0));
    float shadow = 0.0;
    int samples = 2; // Creates 5x5 kernel (total 25 samples)
    int sampleCount = 0;

    // Larger PCF kernel for smoother shadows
    for(int x = -samples; x <= samples; x++) {
        for(int y = -samples; y <= samples; y++) {
            float pcfDepth = texture(SHADOW_MAP, proj.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 0.0 : 1.0;
            sampleCount++;
        }
    }

    return shadow / float(sampleCount);
}

/* ============================================================================
   IBL (Image-Based Lighting) Functions
   ============================================================================ */

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    vec3 maxF0 = max(vec3(1.0 - roughness), F0);
    return F0 + (maxF0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateIBL(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness, float ao) {
    if(USE_IBL == 0) {
        return vec3(0.0);
    }

    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);

    vec3 F = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
    F = mix(F, F * (1.0 - roughness), 0.5);

    vec3 kD = (1.0 - F) * (1.0 - metallic);

    vec3 irradiance = texture(IRRADIANCE_MAP, N).rgb;
    vec3 diffuse = kD * irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    float mipLevel = roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColor = textureLod(PREFILTER_MAP, R, mipLevel).rgb;
    vec3 specular = prefilteredColor * F * (1.0 - roughness * roughness);

    return (diffuse + specular) * ao * 0.7;
}

void main() {

    Material mat = SampleMaterial();

    vec3 N = mat.normal;
    vec3 V = normalize(u_viewPosition - v_frag_pos);

    /* Base reflectivity */
    vec3 F0 = mix(vec3(0.04), mat.albedo.rgb, mat.metallic);

    vec3 Lo = vec3(0.0);

    /* Directional lights */
    for(int i = 0; i < u_directionalLightCount; i++) {
        DirectionalLight light = u_directionalLights[i];

        float shadow = 1.0;
        if(light.castShadows && i == 0) {
            vec3 L = normalize(-light.direction);
            shadow = CalculateShadow(v_frag_pos_light_space, N, L);
        }

        Lo += CalculateDirectionalLight(light, N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness, shadow);
    }

    /* Point lights */
    for(int i = 0; i < u_pointLightCount; i++) {
        Lo += CalculatePointLight(u_pointLights[i], v_frag_pos, N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness);
    }

    /* Spot lights */
    for(int i = 0; i < u_spotLightCount; i++) {
        Lo += CalculateSpotLight(u_spotLights[i], v_frag_pos, N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness);
    }

    /* Ambient term with optional IBL */
    vec3 ambient;
    if(USE_IBL == 1) {
        ambient = CalculateIBL(N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness, mat.ao);
    } else {
        ambient = u_ambientStrength * mat.albedo.rgb * max(mat.ao, 0.1);
    }

    vec3 color = ambient + Lo + mat.emissive;

    // /* Tone mapping (Reinhard) */
    color = color / (color + vec3(1.0));

    // /* Gamma correction */
    color = pow(color, vec3(1.0 / 2.2));

    if(mat.albedo.a < 0.1) {
        discard;
    }

    FRAG_COLOR = vec4(color, mat.albedo.a);
}
