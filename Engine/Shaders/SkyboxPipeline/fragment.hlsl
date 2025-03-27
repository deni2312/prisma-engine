TextureCube g_Texture;
SamplerState g_Texture_sampler; // By convention, texture samplers must use the '_sampler' suffix

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 UV : TEX_COORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in PSInput PSIn,
          out PSOutput PSOut)
{
    float4 Color = g_Texture.Sample(g_Texture_sampler, PSIn.UV);
    PSOut.Color = Color;
}