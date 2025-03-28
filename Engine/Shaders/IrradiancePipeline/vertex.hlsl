// Vertex shader takes two inputs: vertex position and uv coordinates.
// By convention, Diligent Engine expects vertex shader inputs to be 
// labeled 'ATTRIBn', where n is the attribute number.
struct VSInput
{
    float3 Pos : ATTRIB0;
    float2 UV : ATTRIB1;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : TEX_COORD1;
};

cbuffer IBLData
{
    float4x4 view;
    float4x4 projection;
};

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be identical.
void main(in VSInput VSIn,
          out PSInput PSIn)
{
    PSIn.Pos = projection * view * float4(VSIn.Pos, 1.0);
    float3 pos = float3(VSIn.Pos.x, -VSIn.Pos.y, VSIn.Pos.z);
    PSIn.WorldPos = pos;
}