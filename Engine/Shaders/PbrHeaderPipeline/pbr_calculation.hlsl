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
    float3 FragPos : TEX_COORD; // Fragment position in world space
    float4 NormalPS : TEX_COORD;
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

    return normalize(mul(TBN, tangentNormal));
}
