#include "../../../Engine/Shaders/RayTracingPipeline/constants.hlsl"
//#include "../../../Engine/Shaders/RayTracingPipeline/rayutils.hlsl"

//ConstantBuffer<MeshAttribs> g_CubeAttribsCB;

//Texture2D g_CubeTextures[NUM_TEXTURES];
//SamplerState g_SamLinearWrap;

StructuredBuffer<VertexBlas> vertexBlas;
StructuredBuffer<LocationBlas> locationBlas;
StructuredBuffer<int4> primitiveBlas;

[shader("closesthit")]
void main(inout PrimaryRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Calculate triangle barycentrics.
	float3 barycentrics = float3(1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    // Get vertex indices for primitive.
    int3 primitive = primitiveBlas[locationBlas[InstanceID()].locationPrimitive + PrimitiveIndex()].xyz;

    // Calculate texture coordinates.
	/*float2 uv = g_CubeAttribsCB.UVs[primitive.x].xy * barycentrics.x +
                g_CubeAttribsCB.UVs[primitive.y].xy * barycentrics.y +
                g_CubeAttribsCB.UVs[primitive.z].xy * barycentrics.z;*/

    // Calculate and transform triangle normal.
    float3 normal = vertexBlas[locationBlas[InstanceID()].location+primitive.x].norm.xyz * barycentrics.x +
                    vertexBlas[locationBlas[InstanceID()].location+primitive.y].norm.xyz * barycentrics.y +
                    vertexBlas[locationBlas[InstanceID()].location+primitive.z].norm.xyz * barycentrics.z;
	normal = normalize(mul((float3x3) ObjectToWorld3x4(), normal));

    // Sample texturing. Ray tracing shaders don't support LOD calculation, so we must specify LOD and apply filtering.
    payload.Color = normal;
	payload.Depth = RayTCurrent();

    // Apply lighting.
	//float3 rayOrigin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
	//LightingPass(payload.Color, rayOrigin, normal, payload.Recursion + 1);
}