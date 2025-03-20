#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.hlsl"

Texture2D diffuseTexture;
SamplerState diffuseTexture_sampler; // By convention, texture samplers must use the '_sampler' suffix

Texture2D normalTexture;
SamplerState normalTexture_sampler; // By convention, texture samplers must use the '_sampler' suffix

Texture2D rmTexture;
SamplerState rmTexture_sampler; // By convention, texture samplers must use the '_sampler' suffix

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in PSInput PSIn,
          out PSOutput PSOut)
{
    float4 diffuse = diffuseTexture.Sample(diffuseTexture_sampler, PSIn.UV);
    float3 normal = GetNormalFromMap(normalTexture, normalTexture_sampler, PSIn.UV, PSIn.FragPos, (float3)PSIn.NormalPS);
    float4 rm = rmTexture.Sample(rmTexture_sampler, PSIn.UV);

#if CONVERT_PS_OUTPUT_TO_GAMMA
    // Use fast approximation for gamma correction.
    diffuse.rgb = pow(diffuse.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
#endif
    float metallic = rm.b;
    float roughness = rm.g;
    
    float3 color = pbrCalculation(PSIn.FragPos, normal, (float3)diffuse, float4(1.0), roughness, metallic);
        
    PSOut.Color = float4(color, 1);
}