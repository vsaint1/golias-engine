in vec2 tex_coord;
in vec4 color;
uniform sampler2D TEXTURE;
uniform bool use_texture;
out vec4 COLOR;
void main() {
    if (use_texture) {
        vec4 tex_color = texture(TEXTURE, tex_coord);
        COLOR = tex_color * color;
    } else {
        COLOR = color;
    }
}
