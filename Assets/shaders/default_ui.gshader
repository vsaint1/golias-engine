@vertex

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

layout(std140) uniform PerFrame {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    mat4 _OrthoMatrix;
    vec4 _CameraPosition;
    vec4 _ShadowSplits;
    mat4 _ShadowMatrices[4];
    mat4 _ShadowViewProjection;
};

void main() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = _OrthoMatrix * vec4(aPosition, 0.0, 1.0);
}

@fragment

in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D _MainTexture;

out vec4 COLOR;

void main() {
    COLOR = texture(_MainTexture, vTexCoord) * vColor;
}
