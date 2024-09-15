#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

in float Height;

in vec3 FragPos;
in vec3 Normal;

in vec2 textureCoord;

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

const float PI = 3.14159265359;

layout(std430, binding = 10) readonly buffer LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(std430, binding = 5) restrict buffer clusterSSBO
{
    Cluster clusters[];
};

struct DirectionalData
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    sampler2DArray depthMap;
    vec2 padding;
};

layout(std140, binding = 2) uniform ClusterData
{
    uvec4 gridSize;
    uvec4 screenDimensions;
    float zNear;
    float zFar;
    float padding[2];
};

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 3) uniform FragmentData
{
    vec4 viewPos;
    samplerCube irradianceMap;
    samplerCube prefilterMap;
    sampler2D brdfLUT;
    vec2 paddingFragment;
};

struct OmniData {
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 far_plane;
    vec4 attenuation;
    samplerCube depthMap;
    float padding;
    float radius;
};

layout(std430, binding = 2) buffer Directional
{
    vec4 lenDir;
    DirectionalData directionalData[];
};

layout(std430, binding = 3) buffer Omni
{
    vec4 lenOmni;
    OmniData omniData[];
};

layout(std430, binding = 9) buffer CSMShadow
{
    float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

vec3 getNormalFromMap(vec3 tangentNormal)
{
    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(textureCoord);
    vec2 st2 = dFdy(textureCoord);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

float ShadowCalculation(vec3 fragPos, vec3 lightPos, int depthMapId)
{
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos.xyz - fragPos);
    float diskRadius = (1.0 + (viewDistance / omniData[depthMapId].far_plane.r)) / 25.0;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(omniData[depthMapId].depthMap, fragToLight + gridSamplingDisk[depthMapId] * diskRadius).r;
        closestDepth *= omniData[depthMapId].far_plane.r;   // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);


    return shadow;

}

float ShadowCalculationDirectional(vec3 fragPosWorldSpace, vec3 lightPos, vec3 N, unsigned int i)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < sizeCSM; ++i)
    {
        if (depthValue < cascadePlanes[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = int(sizeCSM);
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightPos)), 0.005);
    if (layer == int(sizeCSM))
    {
        bias *= 1 / (farPlaneCSM * directionalData[i].padding.y);
    }
    else
    {
        bias *= 1 / (cascadePlanes[layer] * directionalData[i].padding.y);
    }



    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(directionalData[i].depthMap, 0));
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(directionalData[i].depthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main()
{
    // Normalize height range (-16 to 48) to (0 to 1)
    float normalizedHeight = (Height - shift) / mult;

    // Define height thresholds for different layers
    float grassHeight = 0.3;  // Below 30% height, it's mostly grass
    float rockHeight = 0.7;   // Between 30% and 70%, it's rock
    float snowHeight = 1.0;   // Above 70%, snow

    // Blend factors between layers
    float grassFactor = smoothstep(0.0, grassHeight, normalizedHeight);
    float rockFactor = smoothstep(grassHeight, rockHeight, normalizedHeight);
    float snowFactor = smoothstep(rockHeight, snowHeight, normalizedHeight);

    // Sample textures based on TexCoords
    vec4 grassColor = texture(grass, textureCoord);
    vec4 rockColor = texture(stone, textureCoord);
    vec4 snowColor = texture(snow, textureCoord);

    vec3 grassN = getNormalFromMap(texture(grassNormal, textureCoord).xyz * 2.0 - 1.0);
    vec3 rockN = getNormalFromMap(texture(stoneNormal, textureCoord).xyz * 2.0 - 1.0);
    vec3 snowN = getNormalFromMap(texture(snowNormal, textureCoord).xyz * 2.0 - 1.0);

    float grassR = texture(grass, textureCoord).r;
    float rockR = texture(stone, textureCoord).r;
    float snowR = texture(snow, textureCoord).r;

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

    FragColor = vec4(Lo, 1.0);
}