struct DirectionalData
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    sampler2DArray depthMap;
    vec2 padding;
};

struct OmniData {
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 far_plane;
    vec4 attenuation;
    samplerCube depthMap;
    float padding;
    float radius;
};

struct AreaData
{
    vec4 position[4];
    vec4 diffuse;
    sampler2D depthMap;
    vec2 padding;
};


layout(std430, binding = 2) buffer Directional
{
    ivec4 lenDir;
    DirectionalData directionalData[];
};


layout(std430, binding = 3) buffer Omni
{
    ivec4 lenOmni;
    OmniData omniData[];
};

layout(std430, binding = 31) buffer Area
{
    ivec4 lenArea;
    AreaData areaData[];
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(std430, binding = 5) restrict buffer clusterSSBO
{
    Cluster clusters[];
};

layout(std140, binding = 2) uniform ClusterData
{
    uvec4 gridSize;
    uvec4 screenDimensions;
    float zNear;
    float zFar;
    float padding[2];
};