// Diligent Engine HLSL Shader

struct Cluster
{
    float4 minPoint;
    float4 maxPoint;
    uint count;
    uint lightIndices[100];
};

RWStructuredBuffer<Cluster> clusters;

cbuffer ConstantsClusters
{
    float zNear;
    float zFar;
    float2 padding;
    float4x4 inverseProjection;
    uint4 gridSize;
    uint4 screenDimensions;
};

float3 screenToView(float2 screenCoord);
float3 lineIntersectionWithZPlane(float3 startPoint, float3 endPoint, float zDistance);

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID,
          uint3 GTid : SV_GroupThreadID)
{
    // Eye position is zero in view space
    const float3 eyePos = float3(0.0, 0.0, 0.0);
    uint tileIndex = Gid.x + Gid.y * gridSize.x +
        Gid.z * gridSize.x * gridSize.y;
    float2 tileSize = (float2) screenDimensions / (float2) gridSize.xy;
    
    // calculate the min and max points of a tile in screen space
    float2 minPoint_screenSpace = (float2) Gid.xy * tileSize;
    float2 maxPoint_screenSpace = (float2) (Gid.xy + 1) * tileSize;
    
    // convert them to view space sitting on the near plane
    float3 minPoint_viewSpace = screenToView(minPoint_screenSpace);
    float3 maxPoint_viewSpace = screenToView(maxPoint_screenSpace);
    
    float tileNear =
        zNear * pow(zFar / zNear, Gid.z / (float) gridSize.z);
    float tileFar =
        zNear * pow(zFar / zNear, (Gid.z + 1) / (float) gridSize.z);
    
    // Find the 4 intersection points from the min/max points to this cluster's
    // near and far planes
    float3 minPointNear =
        lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileNear);
    float3 minPointFar =
        lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileFar);
    float3 maxPointNear =
        lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileNear);
    float3 maxPointFar =
        lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileFar);
    
    float3 minPointAABB = min(minPointNear, minPointFar);
    float3 maxPointAABB = max(maxPointNear, maxPointFar);
    
    clusters[tileIndex].minPoint = float4(minPointAABB, 0.0);
    clusters[tileIndex].maxPoint = float4(maxPointAABB, 0.0);
}

// Returns the intersection point of an infinite line and a
// plane perpendicular to the Z-axis
float3 lineIntersectionWithZPlane(float3 startPoint, float3 endPoint, float zDistance)
{
    float3 direction = endPoint - startPoint;
    float3 normal = float3(0.0, 0.0, -1.0); // plane normal
    // skip check if the line is parallel to the plane.
    float t = (zDistance - dot(normal, startPoint)) / dot(normal, direction);
    return startPoint + t * direction; // the parametric form of the line equation
}

float3 screenToView(float2 screenCoord)
{
    // normalize screenCoord to [-1, 1] and
    // set the depth of the coordinate to be on the near plane. This is -1 by
    // default in OpenGL.
    float4 ndc = float4(screenCoord / (float2) screenDimensions * 2.0 - 1.0, -1.0, 1.0);
    float4 viewCoord = inverseProjection*ndc;
    viewCoord = viewCoord / viewCoord.w;
    return viewCoord.xyz;
}
