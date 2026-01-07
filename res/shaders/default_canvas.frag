in vec2 v_uv;
in vec4 v_color;

uniform sampler2D TEXTURE;
uniform int HAS_TEXTURE;

out vec4 FRAG_COLOR;

void main() {

    if(HAS_TEXTURE == 1) {
        FRAG_COLOR = texture(TEXTURE, v_uv) * v_color;
        return;
    }

    FRAG_COLOR = v_color;
}
