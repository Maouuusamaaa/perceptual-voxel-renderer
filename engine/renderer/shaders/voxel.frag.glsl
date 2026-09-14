#version 450
layout(location = 0) flat in uint materialId;
layout(location = 1) in vec3 normal;
layout(location = 0) out vec4 outColor;

void main() {
    float light = max(dot(normalize(normal), normalize(vec3(0.4, 0.8, 0.2))), 0.15);
    float material = float(materialId % 16u) / 15.0;
    outColor = vec4(vec3(material * light), 1.0);
}
