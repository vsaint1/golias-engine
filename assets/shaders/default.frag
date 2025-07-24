
in vec4 vColor;
in vec2 TextureCoords;
in float TextureIndex;

#ifdef GL_ES
    uniform mediump sampler2DArray uTextures;
#else
    uniform sampler2D uTextures[16];
#endif

out vec4 COLOR;

void main() {
#ifdef GL_ES
    vec4 tex = (TextureIndex < 0.5)
    ? vec4(1.0)
    : texture(uTextures, vec3(TextureCoords, TextureIndex));
#else
    vec4 tex = (TextureIndex < 0.5)
    ? vec4(1.0)
    : texture(uTextures[int(TextureIndex) - 1], TextureCoords);
#endif

    COLOR = vColor * tex;
}
