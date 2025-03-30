uniform ConstantsClusters
{
float zNear;
float zFar;
vec2 padding;
mat4 inverseProjection;
ivec4 gridSize;
ivec4 screenDimensions;
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(binding=0)
buffer clusters
{
    Cluster clusters_data[];
};

uniform ViewProjection
{
mat4 view;
mat4 projection;
vec4 viewPos;
};


struct OmniData
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 far_plane;
    vec4 attenuation;
    vec2 depthMap;
    float padding;
    float radius;
};

uniform LightSizes
{
int omniSize;
int dirSize;
int areaSize;
int padding1;
};

readonly buffer omniData{
OmniData omniData_data[];
};


layout ( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

#define _RETURN_ {\
return;}

bool sphereAABBIntersection(vec3 center, float radius, vec3 aabbMin, vec3 aabbMax)
{
    // closest point on the AABB to the sphere center
    vec3 closestPoint = clamp(center, aabbMin, aabbMax);
    // squared distance between the sphere center and closest point
    float distanceSquared = dot(closestPoint - center, closestPoint - center);
    return distanceSquared <= radius * radius;
}

// this just unpacks data for sphereAABBIntersection
bool testSphereAABB(uint i, Cluster cluster)
{
    vec3 center = vec3(view * omniData_data[i].position);
    float radius = omniData_data[i].radius;

    vec3 aabbMin = cluster.minPoint.xyz;
    vec3 aabbMax = cluster.maxPoint.xyz;

    return sphereAABBIntersection(center, radius, aabbMin, aabbMax);
}

void main()
{

    uint lightCount = omniSize;
    uint tileIndex = gl_WorkGroupID.x * 128 + gl_LocalInvocationID.x;
    Cluster cluster = clusters_data[tileIndex];

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
    clusters_data[tileIndex] = cluster;

}
