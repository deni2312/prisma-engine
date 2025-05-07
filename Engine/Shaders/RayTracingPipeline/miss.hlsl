#include "../../../Engine/Shaders/RayTracingPipeline/constants.hlsl"
ConstantBuffer<Constants> g_ConstantsCB;
TextureCube skybox;
SamplerState skybox_sampler;

[shader("miss")]
void main(inout PrimaryRayPayload payload)
{
    if (g_ConstantsCB.raytracingEasy.r == 0)
    {
        float3 rayDir = normalize(WorldRayDirection());
        float3 color = skybox.SampleLevel(skybox_sampler, rayDir, 0).rgb;

        payload.Color = color;
        payload.Depth = g_ConstantsCB.ClipPlanes.y;
    }
    else
    {
        payload.Color = float3(0.0,0.0,0.0);
        payload.Depth = g_ConstantsCB.ClipPlanes.y;
    }
}