#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

layout(bindless_sampler) uniform sampler2D gPosition;
layout(bindless_sampler) uniform sampler2D gNormal;
layout(bindless_sampler) uniform sampler2D gAlbedo;
layout(bindless_sampler) uniform sampler2D gAmbient;

in vec2 TexCoords;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

#include ../HelperHeaderPipeline/light_func.glsl
#include ../PbrHeaderPipeline/pbr_func.glsl
#include ../ShadowHeaderPipeline/shadow_func.glsl
#include ../PbrHeaderPipeline/pbr_calculation.glsl


void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 N = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    vec4 aoSpecular = texture(gAmbient, TexCoords);
    float roughness = texture(gNormal, TexCoords).a;
    float metallic = texture(gAlbedo, TexCoords).a;

    vec3 pbrColor = pbrCalculation(FragPos, N, albedo, aoSpecular,roughness,metallic);

    FragColor = vec4(pbrColor, 1.0);
}
