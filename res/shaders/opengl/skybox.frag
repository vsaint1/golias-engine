
in vec3 UV;
out vec4 COLOR;

uniform samplerCube TEXTURE;

void main() {
    COLOR = texture(TEXTURE, UV);
}