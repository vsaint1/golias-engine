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
uniform sampler2D SHADOW_MAP;

uniform samplerCube IRRADIANCE_MAP;
uniform samplerCube PREFILTER_MAP;
uniform int USE_IBL;

/* ============================================================================
   Texture presence flags
   ============================================================================ */

const int HAS_ALBEDO_FLAG = 1;
const int HAS_METALLIC_FLAG = 2;
const int HAS_ROUGHNESS_FLAG = 4;
const int HAS_NORMAL_FLAG = 8;
const int HAS_AO_FLAG = 16;
const int HAS_EMISSIVE_FLAG = 32;

const float PI = 3.14159265359;

uniform int TEXTURE_FLAGS;

#define TONEMAP_LINEAR   0
#define TONEMAP_REINHARD 1
#define TONEMAP_FILMIC   2
#define TONEMAP_ACES     3
#define TONEMAP_UNCHARTED2 4

uniform int u_tonemap;
uniform float u_exposure;

/* ============================================================================
   Material & Light Structures
   ============================================================================ */

struct Material {
    vec4 albedo;
    float metallic;
    float roughness;
    float ao;
    vec3 emissive;
    vec3 normal;
};

struct MaterialProperties {
    vec4 modulate;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float emissiveStrength;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    bool castShadows;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float range;
    float constant;
    float linear;
    float quadratic;
    bool castShadows;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;
    float innerConeAngle;
    float outerConeAngle;
    float constant;
    float linear;
    float quadratic;
    bool castShadows;
};

uniform MaterialProperties u_material;
uniform vec3 CAMERA_POSITION;
uniform float AMBIENT_STRENGTH;

const int MAX_DIRECTIONAL_LIGHTS = 32;
const int MAX_POINT_LIGHTS = 32;
const int MAX_SPOT_LIGHTS = 32;

uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform int u_directionalLightCount;
uniform PointLight u_pointLights[MAX_POINT_LIGHTS];
uniform int u_pointLightCount;
uniform SpotLight u_spotLights[MAX_SPOT_LIGHTS];
uniform int u_spotLightCount;

/* ============================================================================
   Color Space Conversion (sRGB <-> Linear)
   ============================================================================ */

vec3 SRGBToLinear(vec3 srgb) {
    return mix(srgb / 12.92, pow((srgb + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), srgb));
}

vec3 LinearToSRGB(vec3 linear) {
    return mix(linear * 12.92, pow(linear, vec3(1.0 / 2.4)) * 1.055 - 0.055, step(vec3(0.0031308), linear));
}

vec3 ApplyGamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

/* ============================================================================
   Tonemapping Functions
   ============================================================================ */

vec3 Tonemap_Linear(vec3 c) {
    return c;
}

vec3 Tonemap_Reinhard(vec3 c) {
    return c / (c + vec3(1.0));
}

vec3 Tonemap_Uncharted2(vec3 x) {
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((x * (A * x + C * B) + D * E) /
        (x * (A * x + B) + D * F)) - E / F;
}

vec3 Tonemap_Filmic(vec3 c) {
    c = max(vec3(0.0), c - 0.004);
    return (c * (6.2 * c + 0.5)) / (c * (6.2 * c + 1.7) + 0.06);
}

vec3 Tonemap_ACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 ApplyTonemap(vec3 color) {
    color *= u_exposure;

    if(u_tonemap == TONEMAP_REINHARD)
        color = Tonemap_Reinhard(color);
    else if(u_tonemap == TONEMAP_FILMIC)
        color = Tonemap_Filmic(color);
    else if(u_tonemap == TONEMAP_ACES)
        color = Tonemap_ACES(color);
    else if(u_tonemap == TONEMAP_UNCHARTED2)
        color = Tonemap_Uncharted2(color);
    else
        color = Tonemap_Linear(color);

    return color;
}

/* ============================================================================
   PBR Functions 
   ============================================================================ */

float D_GGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float G_SchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness) {
    return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}

vec3 F_Schlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 F_SchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

/* ============================================================================
   Shadow Calculation
   ============================================================================ */
float CalculateShadow(vec4 fragPosLightSpace, vec3 N, vec3 L) {
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

    if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 1.0;

    float currentDepth = proj.z;
    float NdotL = max(dot(N, L), 0.0);

    float bias = clamp(0.0005 * tan(acos(NdotL)),0.00001, 0.0003);

    vec2 texelSize = 1.0 / vec2(textureSize(SHADOW_MAP, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(SHADOW_MAP, proj.xy + vec2(x,y) * texelSize).r;
            shadow += (currentDepth - bias <= pcfDepth) ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}


Material SampleMaterial() {
    Material mat;

    if((TEXTURE_FLAGS & HAS_ALBEDO_FLAG) != 0) {
        vec4 texColor = texture(ALBEDO_TEXTURE, v_texcoord);
        texColor.rgb = SRGBToLinear(texColor.rgb);
        mat.albedo = texColor * u_material.modulate;
    } else {
        mat.albedo = u_material.modulate;
    }

    mat.albedo.rgb *= SRGBToLinear(v_color);

    mat.metallic = ((TEXTURE_FLAGS & HAS_METALLIC_FLAG) != 0) ? texture(METALLIC_TEXTURE, v_texcoord).b * u_material.metallicFactor : u_material.metallicFactor;
    mat.metallic = clamp(mat.metallic, 0.0, 1.0);

    mat.roughness = ((TEXTURE_FLAGS & HAS_ROUGHNESS_FLAG) != 0) ? texture(ROUGHNESS_TEXTURE, v_texcoord).g * u_material.roughnessFactor : u_material.roughnessFactor;
    mat.roughness = clamp(mat.roughness, 0.04, 1.0);

    mat.ao = ((TEXTURE_FLAGS & HAS_AO_FLAG) != 0) ? texture(AO_TEXTURE, v_texcoord).r : 1.0;

    if((TEXTURE_FLAGS & HAS_EMISSIVE_FLAG) != 0) {
        vec3 emissiveTex = texture(EMISSIVE_TEXTURE, v_texcoord).rgb;
        emissiveTex = SRGBToLinear(emissiveTex);
        mat.emissive = emissiveTex * u_material.emissiveFactor * u_material.emissiveStrength;
    } else {
        mat.emissive = vec3(0.0);
    }

    mat.normal = normalize(v_normal);
    if((TEXTURE_FLAGS & HAS_NORMAL_FLAG) != 0) {
        vec3 tangentNormal = texture(NORMAL_TEXTURE, v_texcoord).xyz * 2.0 - 1.0;
        mat.normal = normalize(mat.normal + tangentNormal * 0.4);
    }

    return mat;
}

/* ============================================================================
   Direct Lighting 
   ============================================================================ */

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

    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(NdotV, NdotL, roughness);
    vec3 F = F_Schlick(HdotV, F0);

    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    vec3 radiance = light.color * light.intensity;

    return (diffuse + specular) * radiance * NdotL * shadow;
}

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
    vec3 L = light.position - fragPos;
    float distance = length(L);

    if(distance > light.range)
        return vec3(0.0);

    L = normalize(L);
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float attenuation = 1.0 / (light.constant + light.linear * distance +
        light.quadratic * distance * distance);
    float rangeFalloff = pow(1.0 - pow(distance / light.range, 4.0), 2.0);
    rangeFalloff = max(rangeFalloff, 0.0);
    attenuation *= rangeFalloff;

    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(NdotV, NdotL, roughness);
    vec3 F = F_Schlick(HdotV, F0);

    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    vec3 radiance = light.color * light.intensity * attenuation;

    return (diffuse + specular) * radiance * NdotL;
}

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
    vec3 L = light.position - fragPos;
    float distance = length(L);

    if(distance > light.range)
        return vec3(0.0);

    L = normalize(L);

    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.innerConeAngle - light.outerConeAngle;
    float spotIntensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
    spotIntensity = smoothstep(0.0, 1.0, spotIntensity);

    if(spotIntensity <= 0.0)
        return vec3(0.0);

    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float attenuation = 1.0 / (light.constant + light.linear * distance +
        light.quadratic * distance * distance);
    float rangeFalloff = pow(1.0 - pow(distance / light.range, 4.0), 2.0);
    rangeFalloff = max(rangeFalloff, 0.0);
    attenuation *= rangeFalloff;

    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(NdotV, NdotL, roughness);
    vec3 F = F_Schlick(HdotV, F0);

    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;

    return (diffuse + specular) * radiance * NdotL;
}

vec3 CalculateIBL(
    vec3 N,
    vec3 V,
    vec3 F0,
    vec3 albedo,
    float metallic,
    float roughness,
    float ao
) {
    if(USE_IBL == 0) {
        return vec3(0.0);
    }

    float NdotV = max(dot(N, V), 0.0);
    vec3 R = reflect(-V, N);

    vec3 F = F_SchlickRoughness(NdotV, F0, roughness);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 irradiance = texture(IRRADIANCE_MAP, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefiltered = textureLod(PREFILTER_MAP, R, roughness * MAX_REFLECTION_LOD).rgb;

    float specularWeight = mix(1.0, 0.0, roughness);
    vec3 specular = prefiltered * F * specularWeight;

    vec3 ambient = (kD * diffuse + specular) * ao;

    return ambient;
}

/* ============================================================================
   Studio Lighting - Simplified for non-PBR look
   ============================================================================ */

vec3 StudioLighting(vec3 N, vec3 V, vec3 albedo, float roughness) {
    vec3 totalLight = vec3(0.0);

    // Key light (main directional light)
    vec3 keyDir = normalize(vec3(0.5, 0.8, 0.6));
    float keyNdotL = max(dot(N, keyDir), 0.0);
    vec3 keyColor = vec3(1.0, 0.98, 0.95) * 1.8;
    totalLight += albedo * keyColor * keyNdotL;

    // Fill light (softer, from opposite side)
    vec3 fillDir = normalize(vec3(-0.3, 0.3, -0.4));
    float fillNdotL = max(dot(N, fillDir), 0.0);
    vec3 fillColor = vec3(0.7, 0.8, 1.0) * 0.4;
    totalLight += albedo * fillColor * fillNdotL;

    // Rim light (edge highlight)
    float rimPower = 1.0 - max(dot(N, V), 0.0);
    rimPower = pow(rimPower, 3.0);
    totalLight += vec3(0.9, 0.95, 1.0) * rimPower * 0.3;

    // Ambient base
    totalLight += albedo * vec3(0.15, 0.16, 0.18);

    return totalLight;
}

void main() {
    Material mat = SampleMaterial();

    vec3 N = mat.normal;
    vec3 V = normalize(CAMERA_POSITION - v_frag_pos);

    vec3 F0 = mix(vec3(0.04), mat.albedo.rgb, mat.metallic);

    vec3 Lo = vec3(0.0);
    bool hasLights = false;

    // Directional lights
    for(int i = 0; i < u_directionalLightCount; i++) {
        hasLights = true;
        DirectionalLight light = u_directionalLights[i];
        float shadow = 1.0;

        if(light.castShadows && i == 0) {
            vec3 L = normalize(-light.direction);
            shadow = CalculateShadow(v_frag_pos_light_space, N, L);
        }

        Lo += CalculateDirectionalLight(light, N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness, shadow);
    }

    // Point lights
    for(int i = 0; i < u_pointLightCount; i++) {
        hasLights = true;
        Lo += CalculatePointLight(u_pointLights[i], v_frag_pos, N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness);
    }

    // Spot lights
    for(int i = 0; i < u_spotLightCount; i++) {
        hasLights = true;
        Lo += CalculateSpotLight(u_spotLights[i], v_frag_pos, N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness);
    }

    // Ambient/IBL
    vec3 ambient;
    if(USE_IBL == 1) {
        // PBR path with IBL
        ambient = CalculateIBL(N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness, mat.ao);
    } else if(!hasLights) {
        // No lights - use simplified studio lighting (non-PBR look)
        Lo = StudioLighting(N, V, mat.albedo.rgb, mat.roughness);
        ambient = vec3(0.0); // Already included in studio lighting
    } else {
        // Has lights but no IBL - use simple hemisphere ambient
        float upDot = dot(N, vec3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
        vec3 skyColor = vec3(0.5, 0.6, 0.7);
        vec3 groundColor = vec3(0.2, 0.22, 0.25);
        vec3 ambientColor = mix(groundColor, skyColor, upDot) * AMBIENT_STRENGTH;
        ambient = ambientColor * mat.albedo.rgb * mat.ao;
    }

    vec3 color = ambient + Lo + mat.emissive;

    color = ApplyTonemap(color);
    color = ApplyGamma(color);

    if(mat.albedo.a < 0.1) {
        discard;
    }

    FRAG_COLOR = vec4(color, mat.albedo.a);
}