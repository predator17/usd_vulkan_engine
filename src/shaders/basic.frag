#version 450

// Input from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

// Output
layout(location = 0) out vec4 outColor;

// Blue metal material - cleaner look
const vec3 BASE_COLOR = vec3(0.2, 0.35, 0.6);   // Muted blue base
const vec3 METAL_COLOR = vec3(0.4, 0.6, 0.9);    // Bright blue highlights
const float METALNESS = 0.9;                      // Metallic
const float ROUGHNESS = 0.35;                      // Semi-matte
const float SPECULAR_STRENGTH = 2.2;              // Strong highlights

// Light positions
const vec3 LIGHT_DIRECTIONS[3] = vec3[3](
    normalize(vec3(1.0, 1.0, 1.0)),    // Top-right-front
    normalize(vec3(-1.0, 0.5, 0.5)),   // Top-left-front
    normalize(vec3(0.0, -1.0, 0.5))    // Bottom-front
);

const vec3 LIGHT_COLORS[3] = vec3[3](
    vec3(1.0, 1.0, 1.0),              // White main light
    vec3(0.8, 0.9, 1.0),              // Cool fill light
    vec3(0.6, 0.7, 0.9)               // Blue-ish accent
);

// Blinn-Phong specular calculation
vec3 specularBlinnPhong(vec3 normal, vec3 lightDir, vec3 viewDir, float shininess) {
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    return vec3(spec);
}

// Fresnel-Schlick approximation for metallic edge effect
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    float x = 1.0 - cosTheta;
    float x2 = x * x;
    float x5 = x2 * x2 * x;
    return F0 + (1.0 - F0) * x5;
}

void main() {
    vec3 normal = normalize(fragNormal);

    // Camera/view direction
    vec3 viewDir = normalize(vec3(0.0, 0.0, 5.0) - fragWorldPos);

    // Fresnel base reflectivity for metals
    vec3 F0 = mix(vec3(0.04), METAL_COLOR, METALNESS);

    // Calculate fresnel effect
    float NdotV = max(dot(normal, viewDir), 0.0);
    vec3 fresnel = fresnelSchlick(NdotV, F0);

    // Accumulate lighting from all sources
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < 3; i++) {
        vec3 lightDir = LIGHT_DIRECTIONS[i];
        vec3 lightColor = LIGHT_COLORS[i];

        // Diffuse (Lambertian)
        float NdotL = max(dot(normal, lightDir), 0.0);
        diffuse += (1.0 - METALNESS) * BASE_COLOR * NdotL * lightColor;

        // Specular (Blinn-Phong)
        float shininess = mix(2.0, 256.0, 1.0 - ROUGHNESS);
        vec3 spec = specularBlinnPhong(normal, lightDir, viewDir, shininess);
        specular += spec * METAL_COLOR * lightColor * SPECULAR_STRENGTH;
    }

    // Rim lighting for metallic edge glow
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, 4.0) * 0.4;
    vec3 rimColor = METAL_COLOR * rim * vec3(0.5, 0.7, 1.0);

    // Combine all components
    vec3 ambient = BASE_COLOR * 0.15;
    vec3 color = ambient + diffuse * 0.35 + specular + rimColor;

    // Tone mapping (simple Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
