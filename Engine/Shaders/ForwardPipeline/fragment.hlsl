#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.hlsl"

Texture2D diffuseTexture[];
SamplerState texture_sampler;

Texture2D normalTexture[];

Texture2D rmTexture[];

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in PSInput PSIn,
          out PSOutput PSOut)
{
    float4 diffuse = diffuseTexture[PSIn.drawId].Sample(texture_sampler, PSIn.UV);
    float3 normal = GetNormalFromMap(normalTexture[PSIn.drawId], texture_sampler, PSIn.UV, PSIn.FragPos, PSIn.NormalPS);

    float4 rm = rmTexture[PSIn.drawId].Sample(texture_sampler, PSIn.UV);

    float metallic = rm.b;
    float roughness = rm.g;
    
    float3 color = pbrCalculation(PSIn.FragPos, normal, (float3)diffuse, float4(1.0), roughness, metallic);
        
    PSOut.Color = float4(color, 1);
}