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
    for (uint i = 0; i < omniSize; i++)
    {
        OmniData light = omniData[i];
        
        diffuse.a = light.diffuse.a + PSIn.FragPos.b+normal.r+rm.a+viewPos.b;
        
    }
    
    diffuse.r = normal.r;
    
    diffuse.g = rm.r;
        
    PSOut.Color = float4(normal, diffuse.a);
}