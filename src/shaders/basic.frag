#version 450

// Input from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

// Output
layout(location = 0) out vec4 outColor;

void main() {
    // Simple directional light from top-right-front
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fragNormal);

    // Ambient
    float ambient = 0.2;

    // Diffuse (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Combine with object color
    vec3 lighting = (ambient + diffuse) * fragColor;

    outColor = vec4(lighting, 1.0);
}
