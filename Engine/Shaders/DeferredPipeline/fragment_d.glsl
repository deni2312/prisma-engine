#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

layout(bindless_sampler) uniform sampler2D gPosition;
layout(bindless_sampler) uniform sampler2D gNormal;
layout(bindless_sampler) uniform sampler2D gAlbedo;
uniform float biasCSM;

layout(std140, binding = 2) uniform ClusterData
{
    uvec4 gridSize;
    uvec4 screenDimensions;
    float zNear;
    float zFar;
    float padding[2];
};

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

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

struct DirectionalData
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    sampler2DArray depthMap;
    vec2 padding;
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

layout(std430, binding = 2) readonly buffer Directional
{
    vec4 lenDir;
    DirectionalData directionalData[];
};

layout(std430, binding = 3) readonly buffer Omni
{
    vec4 lenOmni;
    OmniData omniData[];
};


layout(std430, binding = 9) readonly buffer CSMShadow
{
    float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

layout(std140, binding = 3) uniform FragmentData
{
    vec4 viewPos;
    samplerCube irradianceMap;
    samplerCube prefilterMap;
    sampler2D brdfLUT;
    vec2 paddingFragment;
};

const float PI = 3.14159265359;

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
vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPos,vec3 lightPos,int depthMapId)
{
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos.xyz - fragPos);
    float diskRadius = (1.0 + (viewDistance / omniData[depthMapId].far_plane.r)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(omniData[depthMapId].depthMap, fragToLight + gridSamplingDisk[depthMapId] * diskRadius).r;
        closestDepth *= omniData[depthMapId].far_plane.r;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
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

in vec2 TexCoords;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 N = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float roughness = texture(gNormal, TexCoords).a;
    float metallic = texture(gAlbedo, TexCoords).a;

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
