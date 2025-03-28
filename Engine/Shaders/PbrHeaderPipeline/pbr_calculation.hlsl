SamplerState texture_sampler;


struct OmniData
{
    float4 position;
    float4 diffuse;
    float4 specular;
    float4 far_plane;
    float4 attenuation;
    float2 depthMap;
    float padding;
    float radius;
};

cbuffer LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
    float3 FragPos : TEX_COORD1; // Fragment position in world space
    float3 NormalPS : NORMAL;
    int drawId : TEXCOORD2;
};

cbuffer ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4 viewPos;
};

Texture2D lut;
TextureCube irradiance;
TextureCube prefilter;

StructuredBuffer<OmniData> omniData;

static const float PI = 3.14159265359;

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;

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

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// ----------------------------------------------------------------------------
// Converts tangent-space normal to world-space
float3 GetNormalFromMap(Texture2D normalMap, SamplerState samplerState, float2 TexCoords, float3 WorldPos, float3 Normal)
{
    float3 tangentNormal = normalMap.Sample(samplerState, TexCoords).xyz * 2.0 - 1.0;

    float3 Q1 = ddx(WorldPos);
    float3 Q2 = ddy(WorldPos);
    float2 st1 = ddx(TexCoords);
    float2 st2 = ddy(TexCoords);

    float3 N = normalize(Normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(TBN*tangentNormal);
}

float3 pbrCalculation(float3 FragPos, float3 N, float3 albedo, float4 aoSpecular, float roughness, float metallic)
{
    float3 V = normalize((float3)viewPos - FragPos);
    float3 F0 = float3(0.04);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0);
    for (int i = 0; i < omniSize; ++i)
    {
        float3 position = (float3) omniData[i].position;
        float3 L = normalize(position - FragPos);
        float3 H = normalize(V + L);
        float3 distance = (float3)omniData[i].position - FragPos;
        float totalDistance = length(distance);

        float attenuation = 1.0 / (omniData[i].attenuation.x + omniData[i].attenuation.y * totalDistance + omniData[i].attenuation.z * totalDistance * totalDistance);
        
        float3 radiance = (float3) omniData[i].diffuse * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = float3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    float3 irradianceMap = irradiance.Sample(texture_sampler, N);    
    
    float3 diffuse = irradianceMap * albedo;
    float3 R = reflect(-V, N);

    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = prefilter.SampleLevel(texture_sampler, R, roughness * MAX_REFLECTION_LOD);

    float2 brdf = lut.Sample(texture_sampler, float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    float3 ambient = (kD * diffuse + specular);
    float3 color = ambient + Lo;

    
    return color;
}