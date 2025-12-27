#version 330 core

in vec3 v_color;
in vec2 v_texcoord;

out vec4 fragColor;

uniform sampler2D TEXTURE;

void main() {
    vec4 texColor = texture(TEXTURE, v_texcoord);
    
    fragColor = texColor * vec4(v_color, 1.0);
}