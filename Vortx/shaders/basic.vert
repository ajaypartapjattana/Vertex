#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;

    vec4 lightDir;
    vec4 lightColor;

    float curveStrength;
    
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

void main(){
    vec4 pos = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    gl_Position = vec4(pos.x, pos.y, pos.z, pos.w);

    fragColor = inColor;

    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    vec3 N = normalize(normalMatrix * inNormal);
    vec3 T = normalize(normalMatrix * inTangent.xyz);
    vec3 B = cross(N,T) * inTangent.w;

    fragNormal = N; 
    fragTangent = T;
    fragBitTangent = B;

    fragTexCoord = inTexCoord;
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
}