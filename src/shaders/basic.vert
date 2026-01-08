#version 450

// Vertex input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

// Push constants for transforms
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 proj;
} push;

// Output to fragment shader
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragWorldPos;

void main() {
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    gl_Position = push.proj * push.view * worldPos;
    fragColor = inColor;
    // Transform normal to world space (simplified - assumes non-uniform scale not used)
    fragNormal = mat3(push.model) * inNormal;
    fragWorldPos = worldPos.xyz;
}
