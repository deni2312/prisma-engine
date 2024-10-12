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

    vec3 albedo = vec3(blendedColor);
    float metallic = 0.0;
    float roughness = blendedRoughness;

    vec3 V = normalize(viewPos.xyz - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 R = reflect(-V, N);

    for (int i = 0; i < lenDir.r; i++) {

        vec3 L = normalize(vec3(directionalData[i].direction));
        vec3 H = normalize(V + L);


        vec3 radiance = vec3(directionalData[i].diffuse);


        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;

        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        if (directionalData[i].padding.x < 1.0) {
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
        else {
            Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1 - ShadowCalculationDirectional(FragPos, L, N, i));
        }
    }

    // Locating which cluster this fragment is part of
    uint zTile = uint((log(abs(vec3(view * vec4(FragPos, 1.0)).z) / zNear) * gridSize.z) / log(zFar / zNear));
    vec2 tileSize = screenDimensions.xy / gridSize.xy;
    uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
    uint tileIndex =
        tile.x + (tile.y * gridSize.x) + (tile.z * gridSize.x * gridSize.y);

    uint lightCount = clusters[tileIndex].count;

    for (int i = 0; i < lightCount; i++) {
        uint lightIndex = clusters[tileIndex].lightIndices[i];

        vec3 distance = vec3(omniData[lightIndex].position) - FragPos;
        vec3 L = normalize(distance);
        vec3 H = normalize(V + L);

        float totalDistance = length(distance);

        if (totalDistance <= omniData[lightIndex].radius) {

            vec3 radiance = vec3(omniData[lightIndex].diffuse);


            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;

            vec3 kD = vec3(1.0) - kS;

            kD *= 1.0 - metallic;

            float NdotL = max(dot(N, L), 0.0);

            float attenuation = 1.0 / (omniData[lightIndex].attenuation.x + omniData[lightIndex].attenuation.y * totalDistance + omniData[lightIndex].attenuation.z * totalDistance * totalDistance);

            if (omniData[lightIndex].padding.x < 1.0) {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
            }
            else {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation * (1 - ShadowCalculation(FragPos, vec3(omniData[lightIndex].position), int(lightIndex)));
            }

        }
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = kD * diffuse + specular;
    Lo = ambient + Lo;

    FragColor = vec4(Lo,1.0);
}