#version 300 es
precision highp float;

// Common inputs
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uViewProjection;
uniform float uTime;

// Varyings
out vec4 vColor;
out vec2 vTexCoord;


out vec4 COLOR;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void vertex() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}

void fragment() {

    float hue = fract(vTexCoord.x * 2.0 + vTexCoord.y + uTime * 0.3);
    vec3 rainbow = hsv2rgb(vec3(hue, 0.8, 1.0));

    COLOR = vec4(rainbow, 1.0) * vColor;
}

