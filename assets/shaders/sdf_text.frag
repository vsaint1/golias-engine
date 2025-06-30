in vec2 TextureCoords;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
    float distance = texture(uTexture, TextureCoords).r;
    float smoothing = 0.1;

    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    FragColor = vec4(vColor.rgb, vColor.a * alpha);

}