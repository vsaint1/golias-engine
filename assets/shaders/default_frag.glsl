in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_Tex;
uniform vec4 u_Color;

void main()
{
    vec4 texColor = texture(u_Tex, TexCoord);
    FragColor = texColor * u_Color;

    if (FragColor.a < 0.1) discard;
}