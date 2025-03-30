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
    uint lightCount = (uint) omniSize;
    uint tileIndex = Gid.x * LOCAL_SIZE + GTid.x;
    Cluster cluster = clusters[tileIndex];
    
    // we need to reset count because culling runs every frame.
    // otherwise it would accumulate.
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
    // closest point on the AABB to the sphere center
    float3 closestPoint = clamp(center, aabbMin, aabbMax);
    // squared distance between the sphere center and closest point
    float distanceSquared = dot(closestPoint - center, closestPoint - center);
    return distanceSquared <= radius * radius;
}

// this just unpacks data for sphereAABBIntersection
bool testSphereAABB(uint i, Cluster cluster)
{
    float3 center = view * omniData[i].position;
    float radius = omniData[i].radius;
    float3 aabbMin = cluster.minPoint.xyz;
    float3 aabbMax = cluster.maxPoint.xyz;
    return sphereAABBIntersection(center, radius, aabbMin, aabbMax);
}
