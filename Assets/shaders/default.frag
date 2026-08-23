#version 330 core

out vec4 FragColor;

uniform sampler2D uTexture;

in vec3 vColor;
in vec2 vTexCoord;

void main() {
    vec4 tex = texture(uTexture, vTexCoord);
    FragColor = tex * vec4(vColor, 1.0);
}