#include "../../../Engine/Shaders/RayTracingPipeline/constants.hlsl"

#define EPSILON 1e-3f


cbuffer LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};

struct OmniData
{
    float4 position;
    float4 diffuse;
    float4 specular;
    float4 far_plane;
    float4 attenuation;
    float2 depthMap;
    float padding;
    float radius;
};

StructuredBuffer<DirectionalData> dirData;

StructuredBuffer<OmniData> omniData;

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
// === PBR Utility Functions ===

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom + 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k + 1e-6);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float currentRoughness = 1.0 - roughness;
    return F0 + (max(float3(currentRoughness, currentRoughness, currentRoughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

PrimaryRayPayload CastReflectionRay(float3 origin, float3 direction, uint Recursion,float3 normal)
{
    PrimaryRayPayload payload = { float3(0.0, 0.0, 0.0),0, Recursion + 1 };

    if (Recursion >= g_ConstantsCB.MaxRecurionReflection)
        return payload;

    RayDesc ray;
    ray.Origin = origin + normal * SMALL_OFFSET + direction * EPSILON;
    ray.Direction = direction;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    TraceRay(g_TLAS,
             RAY_FLAG_NONE,
             OPAQUE_GEOM_MASK,
             PRIMARY_RAY_INDEX,
             HIT_GROUP_STRIDE,
             PRIMARY_RAY_INDEX,
             ray,
             payload);

    return payload;
}

float2 Hammersley(int i, int N)
{
    uint bits = asuint(i);
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return float2((float) i / (float) N, float(bits) * 2.3283064365386963e-10); // 2^-32
}

float3 ImportanceSampleCosineHemisphere(float2 xi, float3 N)
{
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt(1.0 - xi.y);
    float sinTheta = sqrt(xi.y);

    float3 H = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    // Build TBN
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangentX = normalize(cross(up, N));
    float3 tangentY = cross(N, tangentX);
    return normalize(tangentX * H.x + tangentY * H.y + N * H.z);
}

float3 RayTracedIrradiance(float3 pos, float3 normal, uint Recursion, int sampleCount)
{
    /*
    float3 irradiance = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < sampleCount; ++i)
    {
        // Generate stratified sample direction in hemisphere
        float2 xi = Hammersley(i, sampleCount);
        float3 sampleDir = ImportanceSampleCosineHemisphere(xi, normal);

        // Cast ray
        PrimaryRayPayload payload = CastReflectionRay(pos, sampleDir, Recursion);

        // Weight by cosine
        float weight = max(dot(normal, sampleDir), 0.0);
        irradiance += payload.Color * weight;
    }

    irradiance /= PI * sampleCount;
    return irradiance;*/
    return float3(0.0, 0.0, 0.0);

}

void LightingPass(inout float3 Color, float3 Pos, float3 Norm, uint Recursion, float metalness, float roughness)
{
    float3 V = normalize(g_ConstantsCB.CameraPos.xyz - Pos);
    float3 albedo = Color;
    float alpha = roughness * roughness;

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);

    float3 Lo = float3(0.0, 0.0, 0.0);

    // === Omni Lights ===
    for (int i = 0; i < omniSize; ++i)
    {
        float3 L = omniData[i].position.xyz - Pos;
        float dist = length(L);
        L = normalize(L);
        float3 H = normalize(V + L);

        float NdotL = max(dot(Norm, L), 0.0);
        float NdotV = max(dot(Norm, V), 0.0);
        float NdotH = max(dot(Norm, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        float3 radiance = omniData[i].diffuse.rgb;

        // Fresnel
        float3 F = fresnelSchlick(VdotH, F0);

        // Specular BRDF
        float NDF = DistributionGGX(NdotH, roughness);
        float G = GeometrySmith(NdotV, NdotL, roughness);
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * NdotV * NdotL + 1e-6;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = 1.0 - kS;
        kD *= 1.0 - metalness;

        float3 att = omniData[i].attenuation.xyz;
        float attenuation = 1.0 / (att.x + att.y * dist + att.z * dist * dist);
        attenuation *= saturate(1.0 - dist / omniData[i].radius);

        float shadow = 1.0;

        float3 light = (kD * albedo / PI + specular) * radiance * NdotL * shadow * attenuation;
        Lo += light;
    }

    // === Directional Lights ===
    for (int i = 0; i < dirSize; ++i)
    {
        float3 L = normalize(dirData[i].direction.xyz);
        float3 H = normalize(V + L);

        float NdotL = max(dot(Norm, L), 0.0);
        float NdotV = max(dot(Norm, V), 0.0);
        float NdotH = max(dot(Norm, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        float shadow = 1.0;
        if (dirData[i].hasShadow)
        {
            RayDesc ray;
            ray.Origin = Pos + Norm * SMALL_OFFSET;
            ray.Direction = L;
            ray.TMin = 0.0;
            ray.TMax = 1000.0;
            shadow = CastShadow(ray, Recursion).Shading;
        }

        float D = DistributionGGX(NdotH, roughness);
        float G = GeometrySmith(NdotV, NdotL, roughness);
        float3 F = fresnelSchlick(VdotH, F0);

        float3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 1e-6;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = 1.0 - kS;
        kD *= 1.0 - metalness;

        float3 irradianceData = dirData[i].diffuse.rgb;
        float3 diffuse = (albedo / PI) * irradianceData;

        float3 light = (kD * diffuse + specular) * NdotL * shadow;
        Lo += light;
    }
    // === Ray-Traced Reflections with Roughness and Metalness ===
    float3 R = reflect(-V, Norm);
    R = normalize(R);

    // Compute Fresnel with roughness-aware version
    float NdotV = max(dot(Norm, V), 0.0);
    float3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;

    float3 reflectionColor = float3(0.0, 0.0, 0.0);
    if (Recursion < g_ConstantsCB.MaxRecurionReflection)
    {
        PrimaryRayPayload reflectionPayload = CastReflectionRay(Pos, R, Recursion,Norm);
        reflectionColor = reflectionPayload.Color;
    }

    // Blend reflection using Fresnel (kS)
    float3 specularReflection = kS * reflectionColor * (1.0 - roughness * roughness);

    // Final output
    Color = Lo + specularReflection;
}