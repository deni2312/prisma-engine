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
    const float3 eyePos = float3(0.0, 0.0, 0.0);

    uint tileIndex = Gid.x + Gid.y * gridSize.x +
                     Gid.z * gridSize.x * gridSize.y;
    float2 tileSize = float2(screenDimensions.xy) / float2(gridSize.xy);

    float2 minPoint_screenSpace = Gid.xy * tileSize;
    float2 maxPoint_screenSpace = (Gid.xy + 1) * tileSize;

    float3 minPoint_viewSpace = screenToView(minPoint_screenSpace);
    float3 maxPoint_viewSpace = screenToView(maxPoint_screenSpace);

    float tileNear = zNear * pow(zFar / zNear, Gid.z / float(gridSize.z));
    float tileFar = zNear * pow(zFar / zNear, (Gid.z + 1) / float(gridSize.z));

    float3 minPointNear = lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileNear);
    float3 minPointFar = lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileFar);
    float3 maxPointNear = lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileNear);
    float3 maxPointFar = lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileFar);

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
    float3 normal = float3(0.0, 0.0, -1.0);

    float t = (zDistance - dot(normal, startPoint)) / dot(normal, direction);
    return startPoint + t * direction;
}

float3 screenToView(float2 screenCoord)
{
    float4 ndc = float4(screenCoord / screenDimensions * 2.0 - 1.0, -1.0, 1.0);

    float4 viewCoord = mul(ndc, inverseProjection);
    viewCoord /= viewCoord.w;
    return viewCoord.xyz;
}
