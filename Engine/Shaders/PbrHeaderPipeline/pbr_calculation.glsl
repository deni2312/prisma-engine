#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require
uniform ConstantsClusters
{
    float zNear;
    float zFar;
    vec2 padding2;
    mat4 inverseProjection;
    ivec4 gridSize;
    ivec4 screenDimensions;
};

struct StatusData
{
    int status;
    int plainMaterial;
    int transparent;
    int isSpecular;
    vec3 GlassReflectionColorMask;
    float GlassAbsorption;
    vec4 GlassMaterialColor;
    vec2 GlassIndexOfRefraction;
    int GlassEnableDispersion;
    int DispersionSampleCount;
};

const float PI = 3.14159265359;

layout(location = 0) out vec4 FragColor;


layout(location = 0) in vec2 outUv;
layout(location = 1) in vec3 outFragPos; // Fragment position in world space
layout(location = 2) in vec3 outNormal;
layout(location = 3) in flat int outDrawId;
//layout(location = 4) in mat3 outTBN;

uniform texture2D diffuseTexture[];

uniform sampler textureClamp_sampler;
uniform sampler textureRepeat_sampler;

uniform texture2D normalTexture[];

readonly buffer statusData{
    StatusData statusData_data[];
};

uniform texture2D rmTexture[];

uniform textureCube omniShadow[];

uniform texture2D lut;
uniform textureCube irradiance;
uniform textureCube prefilter;

uniform texture2DArray csmShadow;

uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
    vec4 cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

struct OmniData
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 far_plane;
    vec4 attenuation;
    vec2 depthMap;
    float padding;
    float radius;
};

struct DirectionalData
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    float hasShadow;
    float bias;
    vec2 padding;
};


uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

uniform LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

readonly buffer omniData{
    OmniData omniData_data[];
};

readonly buffer dirData{
    DirectionalData dirData_data[];
};

layout(binding=0) buffer clusters{
    Cluster clusters_data[];
};



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

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(outFragPos);
    vec3 Q2 = dFdy(outFragPos);
    vec2 st1 = dFdx(outUv);
    vec2 st2 = dFdy(outUv);

    vec3 N = normalize(outNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

const vec3 gridSamplingDisk[20] = vec3[20](
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
    float diskRadius = (1.0 + (viewDistance / omniData_data[depthMapId].far_plane.r)) / 25.0;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(samplerCube(omniShadow[depthMapId],textureClamp_sampler), fragToLight + gridSamplingDisk[depthMapId] * diskRadius).r;
        closestDepth *= omniData_data[depthMapId].far_plane.r;   // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);


    return shadow;

}

float ShadowCalculationDirectional(vec3 fragPosWorldSpace, vec3 lightPos, vec3 N,int i)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < sizeCSM; ++i)
    {
        if (depthValue < cascadePlanes[i].x)
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
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y;

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
        bias *= 1 / (farPlaneCSM * dirData_data[i].bias);
    }
    else
    {
        bias *= 1 / (cascadePlanes[layer].x * dirData_data[i].bias);
    }
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(4096,4096);
    //projCoords.y=1-projCoords.y;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec3 textureLayer=vec3(projCoords.xy + vec2(x, y) * texelSize, layer);
            float pcfDepth = texture(sampler2DArray(csmShadow,textureClamp_sampler), textureLayer).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

vec3 ShadowCalculationDirectionalDebug(vec3 fragPosWorldSpace, vec3 lightPos, vec3 N,int i)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < sizeCSM; ++i)
    {
        if (depthValue < cascadePlanes[i].x)
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
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightPos)), 0.005);
    if (layer == int(sizeCSM))
    {
        bias *= 1 / (farPlaneCSM * dirData_data[i].bias);
    }
    else
    {
        bias *= 1 / (cascadePlanes[layer].x * dirData_data[i].bias);
    }

    return vec3(bias);

}

vec3 pbrCalculation(vec3 FragPos, vec3 N, vec3 albedo, vec4 aoSpecular,float roughness,float metallic) {

    float specularMap = aoSpecular.r;
    float ao = aoSpecular.g;

    vec3 V = normalize(viewPos.xyz - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 R = reflect(-V, N);

    for (int i = 0; i < dirSize; i++) {

        vec3 L = normalize(vec3(dirData_data[i].direction));
        vec3 H = normalize(V + L);


        vec3 radiance = vec3(dirData_data[i].diffuse);


        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F * specularMap;

        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        if (dirData_data[i].hasShadow < 1.0) {
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
        else {
            Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1 - ShadowCalculationDirectional(FragPos, L, N, i));
        }
    }

    // Locating which cluster this fragment is part of
    uint zTile = uint((log(abs(vec3(view * vec4(FragPos, 1.0)).z) / zNear) * gridSize.z) / log(zFar / zNear));
    vec2 tileSize = screenDimensions.xy / gridSize.xy;

    vec2 fragCoord=gl_FragCoord.xy;
    fragCoord.y=screenDimensions.y-fragCoord.y;
    uvec3 tile = uvec3(fragCoord / tileSize, zTile);
    uint tileIndex =
        tile.x + (tile.y * gridSize.x) + (tile.z * gridSize.x * gridSize.y);

    uint lightCount = clusters_data[tileIndex].count;

    for (int i = 0; i < lightCount; i++) {
        uint lightIndex = clusters_data[tileIndex].lightIndices[i];

        vec3 distance = vec3(omniData_data[lightIndex].position) - FragPos;
        vec3 L = normalize(distance);
        vec3 H = normalize(V + L);

        float totalDistance = length(distance);

        if (totalDistance <= omniData_data[lightIndex].radius) {

            vec3 radiance = vec3(omniData_data[lightIndex].diffuse);


            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F * specularMap;

            vec3 kD = vec3(1.0) - kS;

            kD *= 1.0 - metallic;

            float NdotL = max(dot(N, L), 0.0);

            float attenuation = 1.0 / (omniData_data[lightIndex].attenuation.x + omniData_data[lightIndex].attenuation.y * totalDistance + omniData_data[lightIndex].attenuation.z * totalDistance * totalDistance);
            attenuation *= clamp(1.0 - totalDistance / omniData_data[i].radius, 0.0, 1.0);


            if (omniData_data[lightIndex].padding.x < 1.0) {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
            }
            else {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation * (1 - ShadowCalculation(FragPos, vec3(omniData_data[lightIndex].position), int(lightIndex)));
            }

        }
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F * specularMap;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(samplerCube(irradiance,textureClamp_sampler), N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 8.0;
    vec3 prefilteredColor = textureLod(samplerCube(prefilter,textureClamp_sampler), R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(sampler2D(lut,textureClamp_sampler), vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    Lo = ambient + Lo;

    vec3 L = normalize(vec3(dirData_data[0].direction));
    vec3 shadows=ShadowCalculationDirectionalDebug(FragPos, L, N, 0);
    return Lo;
}