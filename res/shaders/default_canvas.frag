in vec2 v_uv;
in vec4 v_color;

uniform sampler2D TEXTURE;

out vec4 FRAG_COLOR;

void main() {

    vec4 tex  = texture(TEXTURE, v_uv) * v_color;

    FRAG_COLOR = tex;
}
