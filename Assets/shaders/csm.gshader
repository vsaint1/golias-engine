@vertex

layout(location = 0) in vec3 aPos;

uniform mat4 _ModelMatrix;
uniform mat4 _ViewMatrix;

void main() {
    gl_Position = _ViewMatrix * _ModelMatrix * vec4(aPos, 1.0);
}

@fragment

void main() {
}