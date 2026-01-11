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

uniform int TEXTURE_FLAGS;

const float PI = 3.14159265359;

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
uniform vec3 u_viewPosition;
uniform float u_ambientStrength;

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
   PBR Functions - 3D Viewer Quality
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
   Soft, High-Quality Shadows
   ============================================================================ */

float CalculateShadow(vec4 fragPosLightSpace, vec3 N, vec3 L) {
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

    if(proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 1.0;

    float currentDepth = proj.z;
    
    // Calculate proper bias based on surface angle to light
    float NdotL = max(dot(N, L), 0.0);
    
    // Use a more aggressive bias strategy to prevent self-shadowing
    float bias = max(0.005 * (1.0 - NdotL), 0.001);
    
    // Additional depth-based bias for far surfaces
    bias += currentDepth * 0.0001;
    
    vec2 texelSize = 1.0 / vec2(textureSize(SHADOW_MAP, 0));
    float shadow = 0.0;
    int count = 0;
    
    // 3x3 PCF for softer shadows
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float pcfDepth = texture(SHADOW_MAP, proj.xy + offset).r;
            shadow += (currentDepth - bias) > pcfDepth ? 0.0 : 1.0;
            count++;
        }
    }
    
    return shadow / float(count);
}

/* ============================================================================
   Material Sampling
   ============================================================================ */

Material SampleMaterial() {
    Material mat;

    mat.albedo = ((TEXTURE_FLAGS & HAS_ALBEDO_FLAG) != 0) 
        ? texture(ALBEDO_TEXTURE, v_texcoord) * u_material.modulate 
        : u_material.modulate;
    
    mat.albedo.rgb *= v_color;

    mat.metallic = ((TEXTURE_FLAGS & HAS_METALLIC_FLAG) != 0) 
        ? texture(METALLIC_TEXTURE, v_texcoord).b * u_material.metallicFactor 
        : u_material.metallicFactor;
    mat.metallic = clamp(mat.metallic, 0.0, 1.0);

    mat.roughness = ((TEXTURE_FLAGS & HAS_ROUGHNESS_FLAG) != 0) 
        ? texture(ROUGHNESS_TEXTURE, v_texcoord).g * u_material.roughnessFactor 
        : u_material.roughnessFactor;
    mat.roughness = clamp(mat.roughness, 0.04, 1.0);

    mat.ao = ((TEXTURE_FLAGS & HAS_AO_FLAG) != 0) 
        ? texture(AO_TEXTURE, v_texcoord).r 
        : 1.0;

    mat.emissive = ((TEXTURE_FLAGS & HAS_EMISSIVE_FLAG) != 0) 
        ? texture(EMISSIVE_TEXTURE, v_texcoord).rgb * u_material.emissiveFactor * u_material.emissiveStrength 
        : vec3(0.0);

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

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 F0, 
                                vec3 albedo, float metallic, float roughness, float shadow) {
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

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 N, vec3 V, vec3 F0,
                         vec3 albedo, float metallic, float roughness) {
    vec3 L = light.position - fragPos;
    float distance = length(L);
    
    if(distance > light.range) return vec3(0.0);
    
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

vec3 CalculateSpotLight(SpotLight light, vec3 fragPos, vec3 N, vec3 V, vec3 F0,
                        vec3 albedo, float metallic, float roughness) {
    vec3 L = light.position - fragPos;
    float distance = length(L);
    
    if(distance > light.range) return vec3(0.0);
    
    L = normalize(L);
    
    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.innerConeAngle - light.outerConeAngle;
    float spotIntensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
    spotIntensity = smoothstep(0.0, 1.0, spotIntensity);
    
    if(spotIntensity <= 0.0) return vec3(0.0);
    
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

/* ============================================================================
   Studio-Quality IBL
   ============================================================================ */

vec3 CalculateIBL(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness, float ao) {
    if(USE_IBL == 0) {
        return vec3(0.0);
    }

    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);

    vec3 F = F_SchlickRoughness(NdotV, F0, roughness);
    vec3 kD = (1.0 - F) * (1.0 - metallic);

    vec3 irradiance = texture(IRRADIANCE_MAP, N).rgb;
    vec3 diffuse = kD * irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(PREFILTER_MAP, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec3 specular = prefilteredColor * F;

    return (diffuse + specular) * ao;
}

/* ============================================================================
   Studio Lighting Setup (when no lights present)
   ============================================================================ */

vec3 StudioLighting(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness) {
    vec3 totalLight = vec3(0.0);
    
    // Key light (main light, slightly warm)
    vec3 keyDir = normalize(vec3(0.5, 0.8, 0.6));
    vec3 keyColor = vec3(1.0, 0.98, 0.95) * 2.2;
    
    vec3 H = normalize(V + keyDir);
    float NdotL = max(dot(N, keyDir), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float HdotV = max(dot(H, V), 0.0);
    
    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(NdotV, NdotL, roughness);
    vec3 F = F_Schlick(HdotV, F0);
    
    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    
    totalLight += (diffuse + specular) * keyColor * NdotL;
    
    // Fill light (softer, cooler, from opposite side)
    vec3 fillDir = normalize(vec3(-0.3, 0.3, -0.4));
    vec3 fillColor = vec3(0.7, 0.8, 1.0) * 0.6;
    NdotL = max(dot(N, fillDir), 0.0);
    totalLight += albedo * fillColor * NdotL * (1.0 - metallic);
    
    // Rim light (edge highlight for depth)
    vec3 rimDir = normalize(vec3(0.0, 0.2, -1.0));
    float rimPower = 1.0 - max(dot(N, V), 0.0);
    rimPower = pow(rimPower, 3.0);
    totalLight += vec3(0.9, 0.95, 1.0) * rimPower * 0.8;
    
    return totalLight;
}

void main() {
    Material mat = SampleMaterial();

    vec3 N = mat.normal;
    vec3 V = normalize(u_viewPosition - v_frag_pos);
    
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
        Lo += CalculatePointLight(u_pointLights[i], v_frag_pos, N, V, F0, 
                                   mat.albedo.rgb, mat.metallic, mat.roughness);
    }

    // Spot lights
    for(int i = 0; i < u_spotLightCount; i++) {
        hasLights = true;
        Lo += CalculateSpotLight(u_spotLights[i], v_frag_pos, N, V, F0, 
                                  mat.albedo.rgb, mat.metallic, mat.roughness);
    }

    // Ambient/IBL
    vec3 ambient;
    if(USE_IBL == 1) {
        ambient = CalculateIBL(N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness, mat.ao);
    } else if(!hasLights) {
        // Studio lighting fallback when no lights present
        Lo = StudioLighting(N, V, F0, mat.albedo.rgb, mat.metallic, mat.roughness);
        ambient = vec3(0.15, 0.16, 0.18) * mat.albedo.rgb * mat.ao; // Subtle ambient
    } else {
        // Gradient ambient for better depth perception
        float upDot = dot(N, vec3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
        vec3 skyColor = vec3(0.5, 0.6, 0.7);
        vec3 groundColor = vec3(0.2, 0.22, 0.25);
        vec3 ambientColor = mix(groundColor, skyColor, upDot) * u_ambientStrength;
        ambient = ambientColor * mat.albedo.rgb * mat.ao;
    }

    vec3 color = ambient + Lo + mat.emissive;

    // Neutral tone mapping for material accuracy
    color = color / (color + vec3(1.0));
    
    // Add subtle contrast boost for 3D viewer look
    color = pow(color, vec3(0.95));
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    if(mat.albedo.a < 0.1) {
        discard;
    }

    FRAG_COLOR = vec4(color, mat.albedo.a);
}