#include "../../../Engine/Shaders/RayTracingPipeline/constants.hlsl"
ConstantBuffer<Constants> g_ConstantsCB;
TextureCube skybox;
SamplerState skybox_sampler;

[shader("miss")]
void main(inout PrimaryRayPayload payload)
{
    float3 rayDir = normalize(WorldRayDirection());
    float3 color = skybox.SampleLevel(skybox_sampler, rayDir,0).rgb;

    payload.Color = color;
    payload.Depth = g_ConstantsCB.ClipPlanes.y;
}