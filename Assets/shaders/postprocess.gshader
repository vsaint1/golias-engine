@vertex

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vUV;

void main() {
    vUV = aTexCoord;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}

@fragment

out vec4 COLOR;

uniform sampler2D _MainTexture; // HDR texture
uniform int _Tonemap;
uniform float _Exposure;

in vec2 vUV;

vec3 reinhard(vec3 v) {
    return v / ( 1.0 + v);
}

vec3 aces_approx(vec3 v)
{
    v *= 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0, 1.0);
}

vec3 gamma_correction(vec3 v) {
    return pow(max(v, vec3(0.0001)), vec3(1.0/2.2));
}


void main() {
    vec3 color = texture(_MainTexture, vUV).rgb;
    color *= _Exposure;

    vec3 mapped = gamma_correction(color);

    if (_Tonemap == 1) {
        mapped = aces_approx(color);
    } else if (_Tonemap == 2) {
        mapped = reinhard(color);
    }

    COLOR = vec4(mapped, 1.0);
}
