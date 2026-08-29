@vertex

layout(location = 0) in vec2 aPosition;
layout(location = 2) in vec4 aColor;
layout(location = 1) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

uniform mat4 _ProjectionMatrix;

void main() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = _ProjectionMatrix * vec4(aPosition, 0.0, 1.0);
}

@fragment

in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D _MainTexture;

out vec4 COLOR;

void main() {
    COLOR = texture(_MainTexture, vTexCoord) * vColor;
}
