
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 aColor;
layout(location=2) in vec2 aTexCoords;
layout(location=3) in float aTexIndex;

uniform mat4 VIEW_PROJECTION;

out vec4 vColor;
out vec2 TextureCoords;
out float TextureIndex;

void main(){
    vColor = aColor;
    TextureCoords = aTexCoords;
    TextureIndex = aTexIndex;
    gl_Position = VIEW_PROJECTION * vec4(aPos, 1.0);
}
