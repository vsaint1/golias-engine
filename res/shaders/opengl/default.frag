out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform vec3 camPos;

// Directional lights
uniform vec3 dirLightDirections[4];
uniform vec3 dirLightColors[4];
uniform bool dirLightCastShadows[4];
uniform int numDirLights;

// Spot lights
uniform vec3 spotLightPositions[4];
uniform vec3 spotLightDirections[4];
uniform vec3 spotLightColors[4];
uniform float spotLightCutOffs[4];
uniform float spotLightOuterCutOffs[4];
uniform int numSpotLights;

// Material
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform vec3 emissive;
uniform float emissiveStrength;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;
uniform sampler2D shadowMap;

uniform bool useAlbedoMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useNormalMap;
uniform bool useAOMap;
uniform bool useEmissiveMap;

const float PI = 3.14159265359;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if(projCoords.z > 1.0)
    shadow = 0.0;

    return shadow;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    if(texture(albedoMap, TexCoord).a < 0.1)
    discard;

    float finalMetallic = 0.0f;
    float finalRoughness = 0.0f;
    if (useMetallicMap) {
        vec3 mr_sample = texture(metallicMap, TexCoord).rgb;
        finalMetallic = mr_sample.b;
        finalRoughness = mr_sample.g;

        if(useRoughnessMap){
            finalRoughness = texture(roughnessMap, TexCoord).r;
        }else{
            finalRoughness = roughness;
        }
    }else{
        finalMetallic = metallic;
    }



    vec3 finalAlbedo = useAlbedoMap ? texture(albedoMap, TexCoord).rgb : albedo;

    float finalAO = useAOMap ? texture(aoMap, TexCoord).r : ao;
    vec3 finalEmissive = useEmissiveMap ? texture(emissiveMap, TexCoord).rgb * emissiveStrength : emissive * emissiveStrength;

    vec3 N = useNormalMap ? getNormalFromMap() : normalize(Normal);
    vec3 V = normalize(camPos - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, finalAlbedo, finalMetallic);

    vec3 Lo = vec3(0.0);

    float shadow = ShadowCalculation(FragPosLightSpace);

    // Directional lights
    for(int i = 0; i < numDirLights; ++i) {
        vec3 L = normalize(-dirLightDirections[i]);
        vec3 H = normalize(V + L);
        vec3 radiance = dirLightColors[i];

        float NDF = DistributionGGX(N, H, finalRoughness);
        float G = GeometrySmith(N, V, L, finalRoughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - finalMetallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);

        float shadowFactor = dirLightCastShadows[i] ? (1.0 - shadow) : 1.0;
        Lo += (kD * finalAlbedo / PI + specular) * radiance * NdotL * shadowFactor;
    }

    // Spot lights
    for(int i = 0; i < numSpotLights; ++i) {
        vec3 L = normalize(spotLightPositions[i] - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(spotLightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance * distance);

        float theta = dot(L, normalize(-spotLightDirections[i]));
        float epsilon = spotLightCutOffs[i] - spotLightOuterCutOffs[i];
        float intensity = clamp((theta - spotLightOuterCutOffs[i]) / epsilon, 0.0, 1.0);

        vec3 radiance = spotLightColors[i] * attenuation * intensity;

        float NDF = DistributionGGX(N, H, finalRoughness);
        float G = GeometrySmith(N, V, L, finalRoughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - finalMetallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * finalAlbedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * finalAlbedo * finalAO;
    vec3 color = ambient + Lo + finalEmissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}