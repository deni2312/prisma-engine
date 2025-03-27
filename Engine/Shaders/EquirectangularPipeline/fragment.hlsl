// Input and output structures
struct PSInput
{
    float4 Pos : SV_POSITION; // Pixel position
    float3 WorldPos : TEX_COORD1;
};

struct PSOutput
{
    float4 Color : SV_TARGET; // Output color (BRDF integration result)
};

static const float2 invAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

Texture2D equirectangularMap;
SamplerState equirectangularMap_sampler;

// Main Pixel Shader function
void main(in PSInput PSIn, out PSOutput PSOut)
{
    float2 uv = SampleSphericalMap(normalize(PSIn.WorldPos));
    float3 color = equirectangularMap.Sample(equirectangularMap_sampler, uv);
    PSOut.Color = float4(color,1);
}