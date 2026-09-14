#version 450

// One invocation per candidate. The CPU path and this shader share the same
// candidate layout conceptually; the production Vulkan backend should bind
// storage buffers for candidates, visibility flags, and compacted draw data.
layout(local_size_x = 64) in;

struct Candidate {
    vec4 position_radius;
    vec4 perceptual; // screenCoverage, importance, lodHint, geometryId
    uint nodeId;
};

layout(std430, set = 0, binding = 0) readonly buffer Candidates { Candidate candidates[]; } inCandidates;
layout(std430, set = 0, binding = 1) writeonly buffer Visibility { uint visible[]; } outVisibility;

layout(push_constant) uniform CullingParams {
    vec4 cameraPosition;
    vec4 cameraForward;
    vec4 frustum; // cosHalfFov, near, far, budget
} params;

void main() {
    uint id = gl_GlobalInvocationID.x;
    if (id >= inCandidates.candidates.length()) return;

    Candidate c = inCandidates.candidates[id];
    vec3 delta = c.position_radius.xyz - params.cameraPosition.xyz;
    float distanceToCamera = length(delta);
    bool valid = distanceToCamera >= params.frustum.y &&
                 distanceToCamera - c.position_radius.w <= params.frustum.z &&
                 distanceToCamera > 0.0001;

    float facing = valid ? dot(delta / distanceToCamera, params.cameraForward.xyz) : -1.0;
    valid = valid && facing >= params.frustum.x;

    outVisibility.visible[id] = valid ? 1u : 0u;
}
