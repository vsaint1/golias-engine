#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aNormals;

uniform mat4 _ModelMatrix;
uniform mat4 _ViewMatrix;
uniform mat4 _ProjectionMatrix;

out vec3 vColor;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vWorldPosition;

void main() {
    vec4 worldPosition = _ModelMatrix * vec4(aPos, 1.0);

    gl_Position = _ProjectionMatrix * _ViewMatrix * worldPosition;
    vColor = aColor;
    vTexCoord = aTexCoord;
    vNormal = mat3(transpose(inverse(_ModelMatrix))) * aNormals;
    vWorldPosition = worldPosition.xyz;
}
