#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in uint inMaterial;

layout(push_constant) uniform Frame {
    mat4 viewProjection;
} frame;

layout(location = 0) flat out uint materialId;
layout(location = 1) out vec3 normal;

void main() {
    gl_Position = frame.viewProjection * vec4(inPosition, 1.0);
    materialId = inMaterial;
    normal = inNormal;
}
