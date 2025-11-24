#version 300 es
precision highp float;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uViewProjection;
uniform float uTime;
uniform sampler2D uTexture;

out vec4 vColor;
out vec2 vTexCoord;


out vec4 COLOR;

void vertex() {
    vColor = aColor;
    vTexCoord = aTexCoord;


    vec3 pos = aPosition;
    pos.z += sin(uTime * 2.0 + aPosition.x * 0.1) * 0.1;

    gl_Position = uViewProjection * vec4(pos, 1.0);
}

void fragment() {
    vec2 uv = vTexCoord;

    float pixelSize = 16.0;
    uv = floor(uv * pixelSize) / pixelSize;

    vec4 texColor = texture(uTexture, uv);


    float scanline = sin(vTexCoord.y * 300.0) * 0.1 + 0.9;

    COLOR = texColor * vColor * scanline;
}

