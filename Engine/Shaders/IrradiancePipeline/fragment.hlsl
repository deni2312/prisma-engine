
struct PSInput
{
    float4 Pos : SV_POSITION; // Pixel position
    float3 WorldPos : TEX_COORD1;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

SamplerState environmentMap_sampler;
TextureCube environmentMap;

static const float PI = 3.14159265359;


void main(in PSInput PSIn, out PSOutput PSOut)
{    
    float3 N = normalize(PSIn.WorldPos);

    float3 irradiance = float3(0.0, 0.0, 0.0);

// Tangent space calculation from origin point
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    for (float phi = 0.0; phi < 2.0 * 3.14159265359; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * 3.14159265359; theta += sampleDelta)
        {
        // Spherical to Cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

        // Tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += environmentMap.Sample(environmentMap_sampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance *= 3.14159265359 * (1.0 / nrSamples);

    PSOut.Color = float4(irradiance, 1.0);

}
