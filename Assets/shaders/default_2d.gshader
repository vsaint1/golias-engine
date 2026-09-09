@vertex
layout(location = 0) in vec2 aPos;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform PerFrame {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    mat4 _OrthoMatrix;
    vec4 _CameraPosition;
    vec4 _ShadowSplits;
    mat4 _ShadowMatrices[4];
    mat4 _ShadowViewProjection;
};

layout(std140) uniform PerObject {
    mat4 _ModelMatrix;
    ivec4 _ObjectFlags;
    vec4 _SpritePivotSize;
    vec4 _SpriteUvBounds;
};

layout(std140) uniform PerMaterial {
    vec4 _BaseColor;
};

out vec2 vUV;

void main() {

    vec2 local = (aPos - _SpritePivotSize.xy) * _SpritePivotSize.zw;
    vUV = mix(_SpriteUvBounds.xy, _SpriteUvBounds.zw, aTexCoord);

    gl_Position = _OrthoMatrix * _ViewMatrix * _ModelMatrix * vec4(local, 0.0, 1.0);
}

@fragment

out vec4 COLOR;


uniform sampler2D _MainTexture;
layout(std140) uniform PerMaterial {
    vec4 _BaseColor;
};
in vec2 vUV;

void main() {
    COLOR = texture(_MainTexture, vUV) * _BaseColor;
}