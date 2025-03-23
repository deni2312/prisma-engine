#extension GL_ARB_shader_draw_parameters : enable

cbuffer Constants
{
    float4x4 g_WorldViewProj;
    float4x4 g_Normal;
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
    float3 Normal : ATTRIB1;
    float2 UV : ATTRIB2;
    float3 Tangent : ATTRIB3;
    float3 Bitangent : ATTRIB4;
};

// Vertex shader output / Pixel shader input
struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
    float3 FragPos : TEX_COORD1; // Fragment position in world space
    float3 NormalPS : NORMAL;
};

StructuredBuffer<float4x4> Models;

void main(in VSInput VSIn,
          out PSInput PSIn)
{
    float4 worldPos = Models[gl_DrawIDARB] * float4(VSIn.Pos, 1.0);
    PSIn.NormalPS = float3(g_Normal * float4(VSIn.Normal, 1.0));
    PSIn.Pos = projection*view*worldPos;
    PSIn.UV = VSIn.UV;
    PSIn.FragPos = worldPos.xyz; // Store world position for fragment shading
}