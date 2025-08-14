#include "../../../Engine/Shaders/RayTracingPipeline/lighting.hlsl"

//#include "../../../Engine/Shaders/RayTracingPipeline/rayutils.hlsl"

//ConstantBuffer<MeshAttribs> g_CubeAttribsCB;

//Texture2D g_CubeTextures[NUM_TEXTURES];
Texture2D diffuseTexture[];

Texture2D normalTexture[];

Texture2D rmTexture[];

SamplerState g_SamLinearWrap;

StructuredBuffer<VertexBlas> vertexBlas;
StructuredBuffer<LocationBlas> locationBlas;
StructuredBuffer<int4> primitiveBlas;
StructuredBuffer<StatusData> statusData;


[shader("closesthit")]
void main(inout PrimaryRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Calculate triangle barycentrics.
	float3 barycentrics = float3(1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    // Get vertex indices for primitive.
    int3 primitive = primitiveBlas[locationBlas[InstanceID()].locationPrimitive + PrimitiveIndex()].xyz;

    // Calculate and transform triangle normal.
    float2 uv = vertexBlas[locationBlas[InstanceID()].location + primitive.x].uv.xy * barycentrics.x +
                    vertexBlas[locationBlas[InstanceID()].location + primitive.y].uv.xy * barycentrics.y +
                    vertexBlas[locationBlas[InstanceID()].location + primitive.z].uv.xy * barycentrics.z;
    
    // Calculate and transform triangle normal.
    float3 normal = vertexBlas[locationBlas[InstanceID()].location+primitive.x].norm.xyz * barycentrics.x +
                    vertexBlas[locationBlas[InstanceID()].location+primitive.y].norm.xyz * barycentrics.y +
                    vertexBlas[locationBlas[InstanceID()].location+primitive.z].norm.xyz * barycentrics.z;
    
        // Calculate and transform triangle normal.
    float3 tangent = vertexBlas[locationBlas[InstanceID()].location + primitive.x].tangent.xyz * barycentrics.x +
                    vertexBlas[locationBlas[InstanceID()].location + primitive.y].tangent.xyz * barycentrics.y +
                    vertexBlas[locationBlas[InstanceID()].location + primitive.z].tangent.xyz * barycentrics.z;
    
        // Calculate and transform triangle normal.
    float3 bitangent = vertexBlas[locationBlas[InstanceID()].location + primitive.x].bitangent.xyz * barycentrics.x +
                    vertexBlas[locationBlas[InstanceID()].location + primitive.y].bitangent.xyz * barycentrics.y +
                    vertexBlas[locationBlas[InstanceID()].location + primitive.z].bitangent.xyz * barycentrics.z;
    
    
	// Transform TBN to world space
    float3x3 objToWorld = (float3x3) ObjectToWorld3x4();
    tangent = normalize(mul(objToWorld, tangent));
    bitangent = normalize(mul(objToWorld, bitangent));
    normal = normalize(mul(objToWorld, normal));

    // Build TBN matrix
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    // Sample and decode normal map
    float3 normalMap = normalTexture[NonUniformResourceIndex(InstanceID())].SampleLevel(g_SamLinearWrap, uv, 0).xyz;
    
    //OpenGL to Vulkan
    //normalMap.y = 1 - normalMap.y;
    normalMap = normalMap * 2.0 - 1.0;

    // Transform to world space
    float3 worldNormal = normalize(mul(normalMap, TBN));

    
    float4 rm = rmTexture[NonUniformResourceIndex(InstanceID())].SampleLevel(g_SamLinearWrap, uv, 0);

    float metallic = rm.b;
    float roughness = rm.g;
    if (statusData[InstanceID()].isSpecular == 1)
    {
        // Assume rm.rgb = specularColor (r,g,b), rm.a = glossiness
        float3 specularColor = rm.rgb;
        float glossiness = rm.a;

        // Convert glossiness to roughness
        roughness = 1.0 - glossiness;

        // Approximate metallic from specular color
        // Metallic = max(specularColor.r, specularColor.g, specularColor.b)
        // This is a simplification — for accurate conversion you'd need IOR and energy conservation, but this is a decent heuristic
        metallic = max(max(specularColor.r, specularColor.g), specularColor.b);
    }
    metallic = saturate(metallic + statusData[InstanceID()].metalness);
    roughness = saturate(roughness + statusData[InstanceID()].roughness);
    // Sample texturing. Ray tracing shaders don't support LOD calculation, so we must specify LOD and apply filtering.
    payload.Color = diffuseTexture[NonUniformResourceIndex(InstanceID())].SampleLevel(g_SamLinearWrap, uv, 0).xyz;

	payload.Depth = RayTCurrent();

    // Apply lighting.
	float3 rayOrigin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    if (g_ConstantsCB.raytracingEasy.r == 0)
    {
        LightingPass(payload.Color, rayOrigin, worldNormal, payload.Recursion + 1, metallic, roughness);
    }
}