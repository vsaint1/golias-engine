@vertex
layout(location = 0) in vec2 aPos;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 _ModelMatrix;
uniform mat4 _ViewMatrix;
uniform mat4 _ProjectionMatrix;

uniform vec2 _LowerLeftUV;
uniform vec2 _UpperRightUV;

uniform vec2 _Pivot;
uniform vec2 _Size;

out vec2 vUV;

void main() {

    vec2 local = (aPos - _Pivot) * _Size;
    vUV = mix(_LowerLeftUV, _UpperRightUV, aTexCoord);

    gl_Position = _ProjectionMatrix * _ViewMatrix * _ModelMatrix * vec4(local, 0.0, 1.0);
}

@fragment

out vec4 COLOR;


uniform sampler2D _MainTexture;
uniform vec4 _BaseColor;

in vec2 vUV;

void main() {
    COLOR = texture(_MainTexture, vUV) * _BaseColor;
}