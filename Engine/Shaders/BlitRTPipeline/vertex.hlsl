struct VSInput
{
    float3 Pos : ATTRIB0;
    float2 UV : ATTRIB1;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
};

void main(in VSInput VSIn,
          out PSInput PSIn)
{
    PSIn.Pos = float4(VSIn.Pos, 1.0);
    PSIn.UV = VSIn.UV;
    PSIn.UV.y = 1 - PSIn.UV.y;
}