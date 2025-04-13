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

Texture2D lut;
TextureCube irradiance;
TextureCube prefilter;
SamplerState skybox_sampler;

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

void LightingPass(inout float3 Color,float3 Pos,float3 Norm,uint Recursion,float metalness,float roughness)
{
    RayDesc ray;
    float3 finalColor = float3(0.0, 0.0, 0.0);
    float3 viewDir = normalize(g_ConstantsCB.CameraPos.xyz - Pos);
    float3 albedo = Color;

    float alpha = roughness * roughness;
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);

    ray.Origin = Pos + Norm * SMALL_OFFSET;
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    // === Direct Lighting ===
    for (int i = 0; i < dirSize; ++i)
    {
        float3 lightDir = normalize(dirData[i].direction.xyz);
        float3 halfVec = normalize(viewDir + lightDir);

        float NdotL = max(dot(Norm, lightDir), 0.0);
        float NdotV = max(dot(Norm, viewDir), 0.0);
        float NdotH = max(dot(Norm, halfVec), 0.0);
        float VdotH = max(dot(viewDir, halfVec), 0.0);

        if (NdotL > 0.0)
        {
            float shadowFactor = 1.0;
            if (dirData[i].hasShadow)
            {
                ray.Direction = lightDir;
                shadowFactor = CastShadow(ray, Recursion).Shading;
            }

            float alpha2 = alpha * alpha;
            float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
            float D = alpha2 / (PI * denom * denom + 1e-6);

            float k = (alpha + 1.0) * (alpha + 1.0) / 8.0;
            float G1V = NdotV / (NdotV * (1.0 - k) + k);
            float G1L = NdotL / (NdotL * (1.0 - k) + k);
            float G = G1V * G1L;

            float3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

            float3 specular = (D * G * F) / (4.0 * NdotL * NdotV + 1e-6);

            float3 diffuseColor = (1.0 - F) * (1.0 - metalness);
            float3 diffuse = (diffuseColor * albedo) / PI;

            float3 lightColor = dirData[i].diffuse.rgb;
            float3 lighting = (diffuse + specular) * lightColor * NdotL * shadowFactor;

            finalColor += lighting;
        }
    }

    // === Image-Based Lighting (IBL) ===
    float3 F = F0 + (1.0 - F0) * pow(1.0 - max(dot(viewDir, Norm), 0.0), 5.0);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;

    // Irradiance (Diffuse)
    float3 irradianceColor = irradiance.SampleLevel(skybox_sampler, Norm,0).rgb;
    float3 diffuseIBL = irradianceColor * albedo;

    // Specular (Prefilter + BRDF LUT)
    float3 R = reflect(-viewDir, Norm);
    R = normalize(R);

    const uint MAX_MIP_LEVEL = 5;
    float mipLevel = roughness * MAX_MIP_LEVEL;
    float3 prefilteredColor = prefilter.SampleLevel(skybox_sampler, R, mipLevel).rgb;

    float2 brdfSample = lut.SampleLevel(skybox_sampler, float2(max(dot(Norm, viewDir), 0.0), roughness),0).rg;
    float3 specularIBL = prefilteredColor * (F * brdfSample.x + brdfSample.y);

    float3 ibl = (kD * diffuseIBL + specularIBL);
    finalColor += ibl;

    finalColor += albedo * 0.02;
    
    Color = finalColor;
}
