#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.hlsl"

Texture2D diffuseTexture;
SamplerState diffuseTexture_sampler; // By convention, texture samplers must use the '_sampler' suffix

Texture2D normalTexture;
SamplerState normalTexture_sampler; // By convention, texture samplers must use the '_sampler' suffix

Texture2D rmTexture;
SamplerState rmTexture_sampler; // By convention, texture samplers must use the '_sampler' suffix

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
    float3 FragPos : TEX_COORD1; // Fragment position in world space
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in PSInput PSIn,
          out PSOutput PSOut)
{
    float4 diffuse = diffuseTexture.Sample(diffuseTexture_sampler, PSIn.UV);
    float4 normal = normalTexture.Sample(normalTexture_sampler, PSIn.UV);
    float4 rm = rmTexture.Sample(rmTexture_sampler, PSIn.UV);

#if CONVERT_PS_OUTPUT_TO_GAMMA
    // Use fast approximation for gamma correction.
    diffuse.rgb = pow(diffuse.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
#endif
    for (uint i = 0; i < omniSize; i++)
    {
        OmniData light = omniData[i];
        
        diffuse.a = light.diffuse.a + PSIn.FragPos.b+normal.a+rm.a;
        
    }
    
    diffuse.r = normal.r;
    
    diffuse.g = rm.r;
        
    PSOut.Color = float4(PSIn.FragPos,diffuse.a);
}