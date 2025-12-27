#version 330 core

in vec3 v_color;
in vec2 v_texcoord;

out vec4 COLOR;

uniform sampler2D TEXTURE;

void main() {
    vec4 tex = texture(TEXTURE, v_texcoord);
    COLOR = tex * vec4(v_color , 1.0);
}