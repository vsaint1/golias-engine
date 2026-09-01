@vertex

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aNormals;
layout(location = 4) in vec3 aTangent;
layout(location = 5) in vec3 aBitangent;

uniform mat4 _ModelMatrix;
uniform mat4 _ViewMatrix;
uniform mat4 _ProjectionMatrix;
uniform vec3 _CameraPos;

out vec3 vColor;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vWorldPosition;
out vec3 vViewPosition; 

void main() {
    vec4 worldPosition = _ModelMatrix * vec4(aPos, 1.0);

    gl_Position = _ProjectionMatrix * _ViewMatrix * worldPosition;
    vColor = aColor;
    vTexCoord = aTexCoord;

    mat3 normalMatrix = mat3(transpose(inverse(_ModelMatrix)));


    vNormal    = normalMatrix * aNormals;
    vTangent   = normalMatrix * aTangent;
    vBitangent = normalMatrix * aBitangent;

    vWorldPosition = worldPosition.xyz;
    vViewPosition = (_CameraPos - worldPosition.xyz);

}

@fragment

out vec4 COLOR;

uniform mat4 _ViewMatrix;

uniform sampler2D _MainTexture;
uniform sampler2D _NormalMap;
uniform vec4 _BaseColor;

uniform sampler2DArray _ShadowMap;
uniform mat4 _ShadowMatrices[4];
uniform float _ShadowSplits[4];

in vec3 vColor;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vWorldPosition;
in vec3 vViewPosition;

#define MAX_LIGHTS 32

struct Light {
    vec4 Position;
    vec4 Direction;
    vec4 ColorIntensity;
    float Range;
    float SpotAngle;
    int Type;
    int IsShadowCaster;
};

layout(std140) uniform Lighting {
    int Count;
    int _Padding0;
    int _Padding1;
    int _Padding2;
    Light Lights[MAX_LIGHTS];
};


vec3 calc_bumped_normal(vec3 normal, vec3 tangent, vec3 bitangent, vec2 texCoord) {
    normal = normalize(normal);

   if (dot(tangent, tangent) <= 1e-6) {
        return normal; 
    }

    mat3 TBN = mat3(1.0f);

    tangent = normalize(tangent - normal * dot(normal, tangent));
    bitangent = bitangent - normal * dot(normal, bitangent);
    bitangent = normalize(bitangent - tangent * dot(tangent, bitangent));
    TBN = mat3(tangent, bitangent, normal);

    vec3 tangentSpaceNormal = texture(_NormalMap, texCoord).xyz * 2.0 - 1.0;
    tangentSpaceNormal.z = sqrt(max(1.0 - dot(tangentSpaceNormal.xy, tangentSpaceNormal.xy), 0.0));

    return normalize(TBN * tangentSpaceNormal);
}

float shadow_compare(vec2 uv, float layer, float reference) {
    vec2 mapSize = vec2(textureSize(_ShadowMap, 0));
    vec2 texel = 1.0 / mapSize;
    vec2 position = uv * mapSize - 0.5;
    vec2 base = floor(position);
    vec2 fraction = fract(position);
    float result = 0.0;

    for(int x = 0; x <= 1; ++x) {
        for(int y = 0; y <= 1; ++y) {
            vec2 sampleUv = (base + vec2(x, y) + 0.5) * texel;
            float depth = texture(_ShadowMap, vec3(sampleUv, layer)).r;
            float weight = (x == 0 ? 1.0 - fraction.x : fraction.x) *
                           (y == 0 ? 1.0 - fraction.y : fraction.y);
            result += (reference <= depth ? 1.0 : 0.0) * weight;
        }
    }

    return result;
}

float shadow_factor(vec3 worldPosition, vec3 normal, vec3 lightDirection) {
    float viewDepth = abs((_ViewMatrix * vec4(worldPosition, 1.0)).z);
    int cascade = 0;
    if(viewDepth > _ShadowSplits[0])
        cascade = 1;

    if(viewDepth > _ShadowSplits[1])
        cascade = 2;

    if(viewDepth > _ShadowSplits[2])
        cascade = 3;

    vec4 shadowPosition = _ShadowMatrices[cascade] * vec4(worldPosition, 1.0);
    shadowPosition.xyz /= shadowPosition.w;
    shadowPosition.z = shadowPosition.z * 0.5 + 0.5;

    if(shadowPosition.z < 0.0 || shadowPosition.z > 1.0)
        return 1.0;

    shadowPosition.xy = shadowPosition.xy * 0.5 + 0.5;
    if(shadowPosition.x < 0.0 || shadowPosition.x > 1.0 || shadowPosition.y < 0.0 || shadowPosition.y > 1.0)
        return 1.0;

    const float kBias = 0.0005;
    const float kNormalBias = 0.001;

    float bias = kBias + kNormalBias * (1.0 - max(dot(normal, lightDirection), 0.0));

    vec2 texelSize = 1.0 / vec2(textureSize(_ShadowMap, 0));
    float visibility = 0.0;
    float totalWeight = 0.0;

    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float weight = (x == 0 ? 2.0 : 1.0) * (y == 0 ? 2.0 : 1.0);
            visibility += shadow_compare(shadowPosition.xy + vec2(x, y) * texelSize,
                                          float(cascade), shadowPosition.z - bias) * weight;
            totalWeight += weight;
        }
    }

    return visibility / totalWeight;
}

void main() {
    vec4 tex = texture(_MainTexture, vTexCoord);

    float alpha = tex.a;

    vec3 normal = calc_bumped_normal(vNormal, vTangent, vBitangent, vTexCoord);

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for(int i = 0; i < Count; ++i) {
        Light light = Lights[i];
        vec3 lightDirection;
        float attenuation = 1.0;

        if(light.Type == 0) {
            lightDirection = normalize(-light.Direction.xyz);
        } else {
            vec3 toLight = light.Position.xyz - vWorldPosition;
            float distanceToLight = length(toLight);
            lightDirection = normalize(toLight);
            attenuation = max(0.0, 1.0 - distanceToLight / light.Range);

            if(light.Type == 2) {
                float cone = dot(normalize(-light.Direction.xyz), lightDirection);
                float cutoff = cos(radians(light.SpotAngle));
                attenuation *= step(cutoff, cone);
            }
        }

        float brightness = max(dot(normal, lightDirection), 0.0);
        vec3 lightColor = light.ColorIntensity.rgb * light.ColorIntensity.a * attenuation;

        // Currently only Directional lights cast shadows
        float shadow = (light.Type == 0 && light.IsShadowCaster != 0) ? shadow_factor(vWorldPosition, normal, lightDirection) : 1.0;
        diffuse += brightness * lightColor * shadow;

        vec3 viewDirection = normalize(vViewPosition);
        vec3 reflectionDirection = reflect(-lightDirection, normal);
        float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 32.0);
        specular += 0.5 * specularAmount * lightColor * shadow;
    }

    vec3 kAmbient = vec3(0.08);

    vec3 result = (diffuse + specular + kAmbient) * tex.rgb * vColor * _BaseColor.rgb;

    alpha *= _BaseColor.a;

    // COLOR = vec4(tex.rgb * vColor * _BaseColor.rgb, alpha); // Albedo pass

    // COLOR = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0); // Normal visualization pass
    
    COLOR = vec4(result, alpha); // Final 
}
