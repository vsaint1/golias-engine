in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main()
{
    vec4 texColor = texture(u_Texture, TexCoord);
    FragColor = texColor * u_Color;

    if (FragColor.a < 0.1) discard;
}