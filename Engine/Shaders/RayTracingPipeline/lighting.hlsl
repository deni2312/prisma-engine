#include "../../../Engine/Shaders/RayTracingPipeline/constants.hlsl"

cbuffer LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};
StructuredBuffer<DirectionalData> dirData;
ConstantBuffer<Constants> g_ConstantsCB;
RaytracingAccelerationStructure g_TLAS;

void GetRayPerpendicular(float3 dir, out float3 outLeft, out float3 outUp)
{
    const float3 a = abs(dir);
    const float2 c = float2(1.0, 0.0);
    const float3 axis = a.x < a.y ? (a.x < a.z ? c.xyy : c.yyx) :
                                    (a.y < a.z ? c.xyx : c.yyx);
    outLeft = normalize(cross(dir, axis));
    outUp = normalize(cross(dir, outLeft));
}

float3 DirectionWithinCone(float3 dir, float2 offset)
{
    float3 left, up;
    GetRayPerpendicular(dir, left, up);
    return normalize(dir + left * offset.x + up * offset.y);
}

ShadowRayPayload CastShadow(RayDesc ray, uint Recursion)
{
    // By default initialize Shading with 0.
    // With RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, only intersection and any-hit shaders are executed.
    // Any-hit shader is not used in this tutorial, intersection shader can not write to payload, 
    // so on intersection payload. Shading is always 0,
    // on miss shader payload.Shading will be initialized with 1.
    // With this flags shadow casting executed as fast as possible.
    ShadowRayPayload payload = { 0.0, Recursion };
    
    // Manually terminate the recusrion as the driver doesn't check the recursion depth.
    if (Recursion >= g_ConstantsCB.MaxRecursion)
    {
        payload.Shading = 1.0;
        return payload;
    }
    TraceRay(g_TLAS, // Acceleration structure
             RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
             OPAQUE_GEOM_MASK, // Instance inclusion mask - only opaque instances are visible
             SHADOW_RAY_INDEX, // Ray contribution to hit group index (aka ray type)
             HIT_GROUP_STRIDE, // Multiplier for geometry contribution to hit 
                                // group index (aka the number of ray types)
             SHADOW_RAY_INDEX, // Miss shader index
             ray,
             payload);
    return payload;
}

void LightingPass(inout float3 Color, float3 Pos, float3 Norm, uint Recursion)
{
    RayDesc ray;
    float3 col = float3(0.0, 0.0, 0.0);

    ray.Origin = Pos + Norm * SMALL_OFFSET;
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    for (int i = 0; i < dirSize; ++i)
    {
        float3 lightDir = normalize(dirData[i].direction.xyz);
        float NdotL = max(0.0, dot(Norm, lightDir)); // Negative because light "comes from" the direction

        if (NdotL > 0.0)
        {
            // Soft shadowing if needed
            /*
            int PCFSamples = Recursion > 1 ? min(1, g_ConstantsCB.ShadowPCF) : g_ConstantsCB.ShadowPCF;
            float shading = 0.0;
            for (int j = 0; j < PCFSamples; ++j)
            {
                float2 offset = float2(g_ConstantsCB.DiscPoints[j / 2][(j % 2) * 2], g_ConstantsCB.DiscPoints[j / 2][(j % 2) * 2 + 1]);
                ray.Direction = DirectionWithinCone(-lightDir, offset * 0.005);
                shading += saturate(CastShadow(ray, Recursion).Shading);
            }
            shading = PCFSamples > 0 ? shading / float(PCFSamples) : 1.0;
            */
            float shadowed = 1;
            if (dirData[i].hasShadow)
            {
                ray.Direction = lightDir;
                shadowed = CastShadow(ray, Recursion).Shading;
            }
            
            col += Color * dirData[i].diffuse.rgb * NdotL * shadowed;
        }
        col += Color * 0.125; // Fake indirect bounce or ambient approximation
    }
    Color = col;
}
