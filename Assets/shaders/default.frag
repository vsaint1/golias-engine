#version 330 core

out vec4 COLOR;

uniform vec3 _CameraPos;

uniform sampler2D _MainTexture;
uniform vec4 _BaseColor;

in vec3 vColor;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPosition;

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

void main() {
    vec4 tex = texture(_MainTexture, vTexCoord);
    vec3 normal = normalize(vNormal);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 ambient = vec3(0.0);

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

        diffuse += brightness * lightColor;

        vec3 viewDirection = normalize(_CameraPos - vWorldPosition);
        vec3 reflectionDirection = reflect(-lightDirection, normal);
        float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 32.0);
        specular += 0.5 * specularAmount * lightColor;

        ambient += 0.4 * lightColor;
    }

    vec3 result = (diffuse + specular + ambient) * tex.rgb * vColor * _BaseColor.rgb;
    COLOR = vec4(result, 1.0);
}
