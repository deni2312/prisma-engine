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

/*float4 pbrCalculation(float3 FragPos, float3 N, float3 albedo, float4 aoSpecular, float roughness, float metallic)
{
    float specularMap = aoSpecular.r;
    float ao = aoSpecular.g;

    float3 V = normalize(viewPos.xyz - FragPos);

    float3 F0 = float3(0.04);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0);

    float3 R = reflect(-V, N);
}*/
