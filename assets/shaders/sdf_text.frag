in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D Texture;
uniform vec4 Color;

void main() {
    const float sdf_step = 0.025;
    float distance = texture(Texture, TexCoord).r;
    float alpha = smoothstep(0.5 - sdf_step, 0.5 + sdf_step, distance);
    FragColor = vec4(Color.rgb, alpha * Color.a);
}