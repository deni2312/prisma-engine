#include "../../../Engine/Shaders/RayTracingPipeline/constants.hlsl"

StructuredBuffer<DirectionalData> dirData;

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

void LightingPass(inout float3 Color, float3 Pos, float3 Norm, uint Recursion)
{
    RayDesc ray;
    float3 col = float3(0.0, 0.0, 0.0);

    ray.Origin = Pos + Norm * SMALL_OFFSET;
    ray.TMin = 0.0;
    ray.TMax = 10000.0; // Arbitrary large value, or use FLT_MAX if supported

    for (int i = 0; i < dirSize; ++i)
    {
        float3 lightDir = -normalize(dirData[i].direction);
        float NdotL = max(0.0, dot(Norm, -lightDir)); // Negative because light "comes from" the direction

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

            //ray.Direction = -lightDir; // Ray goes opposite to light direction
            // Assuming hard shadow test for now
            //bool shadowed = CastShadow(ray, Recursion).Shading < 0.5;
            //
            //if (!shadowed)
            //{
            col += Color * dirData[i].diffuse.rgb * NdotL;
            //}
        }
        col += Color * 0.125; // Fake indirect bounce or ambient approximation
    }

    Color = col;
}
