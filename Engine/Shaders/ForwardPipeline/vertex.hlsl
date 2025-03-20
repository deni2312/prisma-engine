cbuffer Constants
{
    float4x4 g_WorldViewProj;
};

cbuffer ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4 viewPos;
};

// Vertex shader input
struct VSInput
{
    float3 Pos : ATTRIB0;
    float2 UV : ATTRIB1;
};

// Vertex shader output / Pixel shader input
struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
    float3 FragPos : TEX_COORD; // Fragment position in world space
};

void main(in VSInput VSIn,
          out PSInput PSIn)
{
    float4 worldPos = mul(float4(VSIn.Pos, 1.0), g_WorldViewProj);
    PSIn.Pos = mul(mul(worldPos, view), projection);
    PSIn.UV = VSIn.UV;
    PSIn.FragPos = worldPos.xyz; // Store world position for fragment shading
}