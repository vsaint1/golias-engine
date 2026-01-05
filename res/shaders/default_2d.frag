in vec2 v_uv;

out vec4 FRAG_COLOR;

uniform vec4 COLOR;

uniform sampler2D TEXTURE;

void main() {
    vec4 tex = texture(TEXTURE, v_uv) * COLOR;
    
    FRAG_COLOR = tex;
}