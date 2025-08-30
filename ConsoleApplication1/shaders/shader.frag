#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;

    vec4 lightDir;
    vec4 lightColor;
} ubo;


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPos;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main(){
    vec3 N = normalize(fragNormal);
    vec3 LightIn = normalize(ubo.lightDir.xyz);

    float diffuseFactor = max(dot(N, LightIn), 0.0);

    vec3 texColor = texture(texSampler, fragTexCoord).rgb;

    vec3 diffuse = diffuseFactor * ubo.lightColor.rgb * texColor;

    vec3 ambience = 0.05 * texColor;

    outColor = vec4((diffuse + ambience), 1.0);
}