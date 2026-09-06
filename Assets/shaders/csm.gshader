@vertex

layout(location = 0) in vec3 aPos;
layout(location = 6) in uvec4 aJoints;
layout(location = 7) in vec4 aWeights;
layout(location = 8) in mat4 aInstanceMatrix;

uniform mat4 _ModelMatrix;
uniform mat4 _ViewMatrix;
uniform int _IsSkinned;
uniform int _InstanceCount;

layout(std140) uniform JointMatrices {
    mat4 _JointMatrices[1024];
};

mat4 skin_matrix() {
    return _JointMatrices[aJoints.x] * aWeights.x + _JointMatrices[aJoints.y] * aWeights.y + _JointMatrices[aJoints.z] * aWeights.z + _JointMatrices[aJoints.w] * aWeights.w;
}

void main() {
    vec4 localPosition = (_IsSkinned != 0) ? skin_matrix() * vec4(aPos, 1.0) : vec4(aPos, 1.0);
    mat4 modelMatrix = (_InstanceCount > 0) ? aInstanceMatrix : _ModelMatrix;
    gl_Position = _ViewMatrix * modelMatrix * localPosition;
}

@fragment

void main() {
}