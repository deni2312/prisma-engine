#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

in vec3 FragPos;
in vec3 Normal;

layout(bindless_sampler) uniform sampler2D grass;
layout(bindless_sampler) uniform sampler2D stone;
layout(bindless_sampler) uniform sampler2D snow;

layout(bindless_sampler) uniform sampler2D grassNormal;
layout(bindless_sampler) uniform sampler2D stoneNormal;
layout(bindless_sampler) uniform sampler2D snowNormal;

layout(bindless_sampler) uniform sampler2D grassRoughness;
layout(bindless_sampler) uniform sampler2D stoneRoughness;
layout(bindless_sampler) uniform sampler2D snowRoughness;

uniform float mult;
uniform float shift;


layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};


#include ../../../Engine/Shaders/HelperHeaderPipeline/light_func.glsl
#include ../../../Engine/Shaders/PbrHeaderPipeline/pbr_func.glsl
#include ../../../Engine/Shaders/ShadowHeaderPipeline/shadow_func.glsl
#include ../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl


vec3 getNormalFromMap(vec3 tangentNormal)
{
    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    float Height = FragPos.y;
    // Normalize height range (-16 to 48) to (0 to 1)
    float normalizedHeight = Height;

    // Define height thresholds for different layers
    float grassHeight = 0.3;  // Below 30% height, it's mostly grass
    float rockHeight = 0.7;   // Between 30% and 70%, it's rock
    float snowHeight = 1.0;   // Above 70%, snow

    // Blend factors between layers
    float grassFactor = smoothstep(0.0, grassHeight, normalizedHeight);
    float rockFactor = smoothstep(grassHeight, rockHeight, normalizedHeight);
    float snowFactor = smoothstep(rockHeight, snowHeight, normalizedHeight);

    // Sample textures based on TexCoords
    vec4 grassColor = texture(grass, TexCoords);
    vec4 rockColor = texture(stone, TexCoords);
    vec4 snowColor = texture(snow, TexCoords);

    vec3 grassN = getNormalFromMap(texture(grassNormal, TexCoords).xyz * 2.0 - 1.0);
    vec3 rockN = getNormalFromMap(texture(stoneNormal, TexCoords).xyz * 2.0 - 1.0);
    vec3 snowN = getNormalFromMap(texture(snowNormal, TexCoords).xyz * 2.0 - 1.0);

    float grassR = texture(grass, TexCoords).r;
    float rockR = texture(stone, TexCoords).r;
    float snowR = texture(snow, TexCoords).r;

    // Blend textures based on height
    vec4 blendedColor = mix(grassColor, rockColor, rockFactor);  // Grass to Rock blend
    blendedColor = mix(blendedColor, snowColor, snowFactor);     // Rock to Snow blend

    // Blend normals based on height
    vec3 blendedNormal = mix(grassN, rockN, rockFactor); // Grass to Rock blend
    blendedNormal = mix(blendedNormal, snowN, snowFactor);    // Rock to Snow blend

    // Blend roughness based on height
    float blendedRoughness = mix(grassR, rockR, rockFactor); // Grass to Rock blend
    blendedRoughness = mix(blendedRoughness, snowR, snowFactor);    // Rock to Snow blend

    // Additional effects based on normal direction (for snow accumulation on flat surfaces)
    float slopeFactor = dot(Normal, vec3(0.0, 1.0, 0.0));  // How flat the surface is
    slopeFactor = clamp(slopeFactor, 0.0, 1.0);            // Ensure slopeFactor is between 0 and 1

    // Accumulate more snow on flat surfaces
    blendedColor = mix(blendedColor, snowColor, slopeFactor * snowFactor);
    blendedNormal = mix(blendedNormal, snowN, slopeFactor * snowFactor);
    blendedRoughness = mix(blendedRoughness, snowR, slopeFactor * snowFactor);

    vec3 N = normalize(blendedNormal);  // Use the blended normal in lighting calculations

    vec3 pbrColor = pbrCalculation(FragPos, N, blendedColor.xyz, vec4(1.0),blendedRoughness,0.0);
    FragColor = vec4(pbrColor, 1.0);
}