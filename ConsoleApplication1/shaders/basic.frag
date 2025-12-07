#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;

    vec4 lightDir;
    vec4 lightColor;

    float curveStrength;

    int selected;
    vec3 _pad;
} ubo;

layout(push_constant) uniform PushConstants{
    int useTexture;
} pc;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitTangent;

layout(binding = 1) uniform sampler2D colorSampler;
layout(binding = 2) uniform sampler2D normalSampler;

layout(location = 0) out vec4 outColor;

void main(){
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitTangent);
    vec3 N = normalize(fragNormal);

    //texNormal.y = -texNormal.y;    
    
    vec3 LightIn = normalize(ubo.lightDir.xyz);
  
    vec3 diffuse;
    vec3 ambience;

    if(pc.useTexture == 1) {
        mat3 TBN = mat3(T, B, N);
        vec3 texNormal = texture(normalSampler, fragTexCoord).xyz;
        texNormal = texNormal * 2.0 - 1.0;
        vec3 mappedNormal = normalize(TBN * texNormal);
        float diffuseFactor = max(dot(mappedNormal, LightIn), 0.0);
        vec3 texColor = texture(colorSampler, fragTexCoord).rgb;
        diffuse = diffuseFactor * ubo.lightColor.rgb * texColor;
        ambience = 0.05 * texColor;
    }else{
        float diffuseFactor = max(dot(N, LightIn), 0.0);
        diffuse = diffuseFactor * ubo.lightColor.rgb * fragColor;
        ambience = 0.05 * fragColor;
    }

    vec3 finalColor = diffuse + ambience;

    if(ubo.selected == 1){
        finalColor = mix(finalColor, vec3(1.0,1.0,0.0), 0.5);
    }

    outColor = vec4(finalColor, 1.0);
}