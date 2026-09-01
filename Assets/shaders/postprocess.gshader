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
// uniform int _Tonemap;
uniform float _Exposure;

in vec2 vUV;

void main() {
    vec3 color = texture(_MainTexture, vUV).rgb;
    color *= _Exposure;

    COLOR = vec4(color, 1.0);
}
