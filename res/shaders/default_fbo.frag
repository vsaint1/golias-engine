in vec2 tex_coord;
out vec4 FRAG_COLOR;

uniform sampler2D TEXTURE;

void main() {

    FRAG_COLOR = texture(TEXTURE, tex_coord);
}
