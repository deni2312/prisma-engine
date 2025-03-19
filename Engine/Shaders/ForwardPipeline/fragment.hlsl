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

/*cbuffer LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};*/

Texture2D g_Texture;
SamplerState g_Texture_sampler; // By convention, texture samplers must use the '_sampler' suffix

StructuredBuffer<OmniData> omniData;

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in PSInput PSIn,
          out PSOutput PSOut)
{
    float4 Color = g_Texture.Sample(g_Texture_sampler, PSIn.UV);
#if CONVERT_PS_OUTPUT_TO_GAMMA
    // Use fast approximation for gamma correction.
    Color.rgb = pow(Color.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
#endif
    for (uint i = 0; i < 1; i++)
    {
        OmniData light = omniData[i];
        
        Color += light.diffuse;
    }
    
    PSOut.Color = Color;
}