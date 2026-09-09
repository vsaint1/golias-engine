@vertex

layout(location = 0) in vec3 aPos;
layout(location = 6) in uvec4 aJoints;
layout(location = 7) in vec4 aWeights;
layout(location = 8) in mat4 aInstanceMatrix;

layout(std140) uniform PerFrame {
    mat4 _ViewMatrix;
    mat4 _ProjectionMatrix;
    mat4 _OrthoMatrix;
    vec4 _CameraPos;
    vec4 _ShadowSplits;
    mat4 _ShadowMatrices[4];
    mat4 _ShadowViewProjection;
};

layout(std140) uniform PerObject {
    mat4 _ModelMatrix;
    ivec4 _ObjectFlags;
    vec4 _ObjectParameters0;
    vec4 _ObjectParameters1;
};

layout(std140) uniform JointMatrices {
    mat4 _JointMatrices[1024];
};

mat4 skin_matrix() {
    return _JointMatrices[aJoints.x] * aWeights.x + _JointMatrices[aJoints.y] * aWeights.y + _JointMatrices[aJoints.z] * aWeights.z + _JointMatrices[aJoints.w] * aWeights.w;
}

void main() {
    vec4 localPosition = (_ObjectFlags.y != 0) ? skin_matrix() * vec4(aPos, 1.0) : vec4(aPos, 1.0);
    mat4 modelMatrix = (_ObjectFlags.x > 0) ? aInstanceMatrix : _ModelMatrix;
    gl_Position = _ShadowViewProjection * modelMatrix * localPosition;
}

@fragment

void main() {
}