in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;

void main() {

    vec4 tex_color  = texture(Texture, TexCoord) * Color;

    FragColor = tex_color;

    if(FragColor.a < 0.1)
        discard;
}