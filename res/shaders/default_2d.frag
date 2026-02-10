in vec2 v_uv;
in vec4 v_color;

out vec4 FRAG_COLOR;

uniform sampler2D TEXTURE;

void main() {
    vec4 tex = texture(TEXTURE, v_uv) * v_color;
    
    FRAG_COLOR = tex;
}