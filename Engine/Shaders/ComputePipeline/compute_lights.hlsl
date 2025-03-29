#define LOCAL_SIZE 128


struct Cluster
{
    float4 minPoint;
    float4 maxPoint;
    uint count;
    uint lightIndices[100];
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

cbuffer LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};

StructuredBuffer<OmniData> omniData;
RWStructuredBuffer<Cluster> clusters;

cbuffer ViewProjection
{
    float4x4 view;
    float4x4 projection;
    float4 viewPos;
};

bool testSphereAABB(uint i, Cluster c);

[numthreads(LOCAL_SIZE, 1, 1)]
void main(uint3 Gid : SV_GroupID,
          uint3 GTid : SV_GroupThreadID)
{
    uint lightCount = (uint) omniSize; // Assuming first element stores light count
    uint tileIndex = Gid.x * LOCAL_SIZE + GTid;

    Cluster cluster = clusters[tileIndex];

    // Reset light count for this frame
    cluster.count = 0;

    for (uint i = 0; i < lightCount; ++i)
    {
        if (testSphereAABB(i, cluster) && cluster.count < 100)
        {
            cluster.lightIndices[cluster.count] = i;
            cluster.count++;
        }
    }

    clusters[tileIndex] = cluster;
}

bool sphereAABBIntersection(float3 center, float radius, float3 aabbMin, float3 aabbMax)
{
    // Closest point on the AABB to the sphere center
    float3 closestPoint = clamp(center, aabbMin, aabbMax);

    // Squared distance between the sphere center and closest point
    float3 delta = closestPoint - center;
    float distanceSquared = dot(delta, delta);

    return distanceSquared <= radius * radius;
    return true;
}

bool testSphereAABB(uint i, Cluster cluster)
{
    float3 center = mul(float4(omniData[i].position.xyz, 1.0), view).xyz;
    float radius = omniData[i].radius;

    float3 aabbMin = cluster.minPoint.xyz;
    float3 aabbMax = cluster.maxPoint.xyz;

    return sphereAABBIntersection(center, radius, aabbMin, aabbMax);
    return true;
}
