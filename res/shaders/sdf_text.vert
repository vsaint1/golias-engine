
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 VIEW_PROJECTION;

out vec2 TextureCoords;
out vec4 vColor;

void main() {
    gl_Position = VIEW_PROJECTION * vec4(aPos, 1.0);
    TextureCoords = aTexCoord;
    vColor = aColor;
}
