in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main() {

    vec4 texColor;

    if(TexCoord == vec2(0.0, 0.0)) {
        texColor = u_Color;
    } else {
        texColor = texture(u_Texture, TexCoord) * u_Color;
    }

    FragColor = texColor;

    if(FragColor.a < 0.1)
        discard;
}