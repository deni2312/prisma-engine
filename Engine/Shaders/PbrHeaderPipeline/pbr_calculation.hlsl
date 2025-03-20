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
};

cbuffer ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4 viewPos;
};

StructuredBuffer<OmniData> omniData;

static const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float DistributionGGX(float3 N, float3 H, float roughness)
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
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
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
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = float3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.03) * albedo * aoSpecular.r;
    float3 color = ambient + Lo;

    color = color / (color + float3(1.0));
    color = pow(color, float3(1.0 / 2.2));
    
    return color;
}