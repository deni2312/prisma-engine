struct PSInput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be identical.
void main(in PSInput PSIn,
          out PSOutput PSOut)
{
    float4 Color = PSIn.Color;
#if CONVERT_PS_OUTPUT_TO_GAMMA
    // Use fast approximation for gamma correction.
    Color.rgb = pow(Color.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
#endif
    PSOut.Color = Color;
}