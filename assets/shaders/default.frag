
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;

void main() {
    bool invalid_uv = TexCoord == vec2(0.0);

    vec4 texColor = invalid_uv ? vec4(1.0) : texture(Texture, TexCoord);
    vec4 base = texColor * Color;

    if (invalid_uv && Color.a < 0.05)
        discard;

    FragColor = base;
}
