@vertex

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aNormals;
layout(location = 4) in vec3 aTangent;
layout(location = 5) in vec3 aBitangent;
layout(location = 6) in uvec4 aJoints;
layout(location = 7) in vec4 aWeights;
layout(location = 8) in mat4 aInstanceMatrix;
layout(location = 12) in vec4 aInstanceColor;

layout(std140) uniform PerFrame {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    mat4 _OrthoMatrix;
    vec4 _CameraPos;
    vec4 _ShadowSplits;
    mat4 _ShadowMatrices[4];
    mat4 _ShadowViewProjection;
};

layout(std140) uniform PerObject {
    mat4 _ModelMatrix;
    ivec4 _ObjectFlags;
    vec4 _SpritePivotSize;
    vec4 _SpriteUvBounds;
    mat3 _NormalMatrix;
 
};

layout(std140) uniform JointMatrices {
    mat4 _JointMatrices[256];
};

out vec3 vColor;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vWorldPosition;
out vec3 vViewPosition;
out vec4 vInstanceColor;
flat out int vInstanced;

mat4 skin_matrix() {
    return _JointMatrices[aJoints.x] * aWeights.x + _JointMatrices[aJoints.y] * aWeights.y + _JointMatrices[aJoints.z] * aWeights.z + _JointMatrices[aJoints.w] * aWeights.w;
}

void main() {
    vInstanced = (_ObjectFlags.x > 0) ? 1 : 0;
    bool isSkinned = _ObjectFlags.y != 0;

    if (vInstanced != 0) {
        vInstanceColor = aInstanceColor;
    } else {
        vInstanceColor = vec4(1.0);
    }

    mat4 modelMatrix = (_ObjectFlags.x > 0) ? aInstanceMatrix : _ModelMatrix;

    mat4 skinMatrix = mat4(1.0);
    if(isSkinned) {
        skinMatrix = skin_matrix();
    }

    vec4 localPosition = isSkinned ? skinMatrix * vec4(aPos, 1.0) : vec4(aPos, 1.0);
    vec4 worldPosition = modelMatrix * localPosition;

    gl_Position = _ProjectionMatrix * _ViewMatrix * worldPosition;
    vColor = aColor;
    vTexCoord = aTexCoord;

    mat3 objectNormalMatrix = _NormalMatrix;
    mat3 normalMatrix = (vInstanced != 0) ? mat3(transpose(inverse(modelMatrix))) : objectNormalMatrix;
    if(isSkinned) {
        normalMatrix *= mat3(skinMatrix);
    }

    vNormal = normalMatrix * aNormals;
    vTangent = normalMatrix * aTangent;
    vBitangent = normalMatrix * aBitangent;

    vWorldPosition = worldPosition.xyz;
    vViewPosition = (_CameraPos.xyz - worldPosition.xyz);

}

@fragment

out vec4 COLOR;

layout(std140) uniform PerFrame {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    mat4 _OrthoMatrix;
    vec4 _CameraPos;
    vec4 _ShadowSplits;
    mat4 _ShadowMatrices[4];
    mat4 _ShadowViewProjection;
};

uniform sampler2D _MainTexture;
uniform sampler2D _NormalMap;
layout(std140) uniform PerMaterial {
    vec4 _BaseColor;
};

uniform mediump sampler2DArray _ShadowMap;

in vec3 vColor;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vWorldPosition;
in vec3 vViewPosition;
in vec4 vInstanceColor;
flat in int vInstanced;

#define MAX_LIGHTS 32

struct Light {
    vec4 Position;
    vec4 DirectionInvRange;
    vec4 ColorIntensity;
    float SpotCutoff;
    int Type;
    int IsShadowCaster;
    int Padding;
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

    if(dot(tangent, tangent) <= 1e-6) {
        return normal;
    }

    tangent = normalize(tangent - normal * dot(normal, tangent));
    bitangent = bitangent - normal * dot(normal, bitangent);
    bitangent = normalize(bitangent - tangent * dot(tangent, bitangent));
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec3 tangentSpaceNormal = texture(_NormalMap, texCoord).xyz * 2.0 - 1.0;
    tangentSpaceNormal.z = sqrt(max(1.0 - dot(tangentSpaceNormal.xy, tangentSpaceNormal.xy), 0.0));

    return normalize(TBN * tangentSpaceNormal);
}

float shadow_compare(vec2 uv, float layer, float reference, vec2 texelSize) {
    vec2 position = uv / texelSize - 0.5;
    vec2 base = floor(position);
    vec2 fraction = fract(position);
    float result = 0.0;

    for(int x = 0; x <= 1; ++x) {
        for(int y = 0; y <= 1; ++y) {
            vec2 sampleUv = (base + vec2(x, y) + 0.5) * texelSize;
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
    if(viewDepth > _ShadowSplits.x)
        cascade = 1;

    if(viewDepth > _ShadowSplits.y)
        cascade = 2;

    if(viewDepth > _ShadowSplits.z)
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
                                         float(cascade),
                                         shadowPosition.z - bias,
                                         texelSize) * weight;
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
    vec3 viewDirection = normalize(vViewPosition);

    for(int i = 0; i < Count; ++i) {
        Light light = Lights[i];
        vec3 lightDirection;
        vec3 toLight = vec3(0.0);
        float attenuation = 1.0;

        if(light.Type == 0) {
            lightDirection = -light.DirectionInvRange.xyz;
        } else {
            toLight = light.Position.xyz - vWorldPosition;
            float distanceToLight = length(toLight);
            lightDirection = normalize(toLight);
            attenuation = max(0.0, 1.0 - distanceToLight * light.DirectionInvRange.w);

            if(light.Type == 2) {
                float cone = dot(-light.DirectionInvRange.xyz, lightDirection);
                attenuation *= step(light.SpotCutoff, cone);
            }
        }

        float brightness = max(dot(normal, lightDirection), 0.0);
        vec3 lightColor = light.ColorIntensity.rgb * light.ColorIntensity.a * attenuation;

        float shadow = 1.0;
        if(light.IsShadowCaster != 0 && light.Type == 0)
            shadow = shadow_factor(vWorldPosition, normal, lightDirection);

        diffuse += brightness * lightColor * shadow;

        vec3 reflectionDirection = reflect(-lightDirection, normal);
        float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 32.0);
        specular += 0.5 * specularAmount * lightColor * shadow;
    }

    vec3 kAmbient = vec3(0.08);

    vec4 baseColor = (vInstanced != 0) ? vInstanceColor : _BaseColor;

    vec3 result = (diffuse + specular + kAmbient) * tex.rgb * vColor * baseColor.rgb;

    alpha *= baseColor.a;

    // COLOR = vec4(tex.rgb * vColor * _BaseColor.rgb, alpha); // Albedo pass

    // COLOR = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0); // Normal visualization pass

    COLOR = vec4(result, alpha); // Final 
}
