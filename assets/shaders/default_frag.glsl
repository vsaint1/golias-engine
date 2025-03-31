in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;

void main() {

    vec4 texColor;

    if(TexCoord == vec2(0.0, 0.0)) {
        texColor = Color;
    } else {
        texColor = texture(Texture, TexCoord) * Color;
    }

    FragColor = texColor;

    if(FragColor.a < 0.1)
        discard;
}