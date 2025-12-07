#version 450

const float PI = 3.14159265358979323846;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;

    vec4 lightDir;
    vec4 lightColor;

    vec4 cameraPos;

    float worldWidth;
    float worldDepth;
    float curveStrength;
    float radius;

    int selected;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitTangent;

void main() {
    vec3 center = vec3(ubo.cameraPos.x, ubo.cameraPos.y - ubo.radius, ubo.cameraPos.z);
    vec3 local = inPosition - center;

    float lon = (local.x / ubo.worldWidth) * PI;
    float lat = (local.z / ubo.worldDepth) * (PI * 0.5);

    vec3 spherePos;
    spherePos.x = ubo.radius * cos(lat) * sin(lon);
    spherePos.y = ubo.radius * sin(lat);
    spherePos.z = ubo.radius * cos(lat) * cos(lon);

    vec3 finalPos = mix(inPosition, spherePos, ubo.curveStrength);

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(finalPos, 1.0);

    fragColor = inColor;

    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    vec3 N = normalize(normalMatrix * inNormal);
    vec3 T = normalize(normalMatrix * inTangent.xyz); 
    vec3 B = cross(N, T) * inTangent.w;

    fragNormal = N;
    fragTangent = T;
    fragBitTangent = B;

    fragTexCoord = inTexCoord;

    // World-space position (using original mesh pos)
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
}
