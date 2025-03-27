// Input and output structures
struct PSInput
{
    float4 Pos : SV_POSITION; // Pixel position
    float2 UV : TEX_COORD; // Texture coordinates for lookup
};

struct PSOutput
{
    float4 Color : SV_TARGET; // Output color (BRDF integration result)
};

Texture2D equirectangularMap;
SamplerState equirectangularMap_sampler;

// Main Pixel Shader function
void main(in PSInput PSIn, out PSOutput PSOut)
{
    PSOut.Color = equirectangularMap.Sample(equirectangularMap_sampler, PSIn.UV);
}