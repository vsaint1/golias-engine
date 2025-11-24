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
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}

void fragment() {
    vec2 uv = vTexCoord;

    uv.x += sin(uv.y * 10.0 + uTime * 3.0) * 0.05;
    uv.y += cos(uv.x * 10.0 + uTime * 2.0) * 0.03;

    vec4 texColor = texture(uTexture, uv);
    COLOR = texColor * vColor;
}

