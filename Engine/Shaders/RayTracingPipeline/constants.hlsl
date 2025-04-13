
struct VertexBlas
{
    float4 norm;
    float4 uv;
};

struct LocationBlas
{
    int location;
    int size;
    int locationPrimitive;
    float padding;
};

struct PrimaryRayPayload
{
    float3 Color;
    float Depth;
    uint Recursion;
};

struct DirectionalData
{
    float4 direction;
    float4 diffuse;
    float4 specular;
    float hasShadow;
    float bias;
    float2 padding;
};

struct ShadowRayPayload
{
    float Shading; // 0 - fully shadowed, 1 - fully in light, 0..1 - for semi-transparent objects
    uint Recursion; // Current recusrsion depth
};

struct StatusData
{
    unsigned int status;
    int plainMaterial;
    bool transparent;
    float padding;
    float3 GlassReflectionColorMask;
    float GlassAbsorption;
    float4 GlassMaterialColor;
    float2 GlassIndexOfRefraction; // min and max IOR
    int GlassEnableDispersion;
    unsigned int DispersionSampleCount; // 1..16
};

#define NUM_LIGHTS          2
#define MAX_DISPERS_SAMPLES 16

struct Constants
{
    // Camera world position
    float4 CameraPos;
    float4x4 InvViewProj;

    // Near and far clip plane distances
    float2 ClipPlanes;
    float MaxRecursion;
    float Padding;
};

struct BoxAttribs
{
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float padding0, padding1;

#ifdef __cplusplus
    BoxAttribs() {}

    BoxAttribs(float _minX, float _minY, float _minZ,
               float _maxX, float _maxY, float _maxZ) :
        minX{_minX}, minY{_minY}, minZ{_minZ},
        maxX{_maxX}, maxY{_maxY}, maxZ{_maxZ},
        padding0{0.0f}, padding1{0.0f}
    {}
#endif
};

static const float4 DispersionSamples[16] =
{
    float4(0.140000f, 0.000000f, 0.266667f, 0.53f),
    float4(0.130031f, 0.037556f, 0.612267f, 0.25f),
    float4(0.100123f, 0.213556f, 0.785067f, 0.16f),
    float4(0.050277f, 0.533556f, 0.785067f, 0.00f),
    float4(0.000000f, 0.843297f, 0.619682f, 0.13f),
    float4(0.000000f, 0.927410f, 0.431834f, 0.38f),
    float4(0.000000f, 0.972325f, 0.270893f, 0.27f),
    float4(0.000000f, 0.978042f, 0.136858f, 0.19f),
    float4(0.324000f, 0.944560f, 0.029730f, 0.47f),
    float4(0.777600f, 0.871879f, 0.000000f, 0.64f),
    float4(0.972000f, 0.762222f, 0.000000f, 0.77f),
    float4(0.971835f, 0.482222f, 0.000000f, 0.62f),
    float4(0.886744f, 0.202222f, 0.000000f, 0.73f),
    float4(0.715967f, 0.000000f, 0.000000f, 0.68f),
    float4(0.459920f, 0.000000f, 0.000000f, 0.91f),
    float4(0.218000f, 0.000000f, 0.000000f, 0.99f)
};


struct ProceduralGeomIntersectionAttribs
{
    float3 Normal;
};

// Instance mask.
#define OPAQUE_GEOM_MASK      0x01
#define TRANSPARENT_GEOM_MASK 0x02

// Ray types
#define HIT_GROUP_STRIDE  2
#define PRIMARY_RAY_INDEX 0
#define SHADOW_RAY_INDEX  1


#ifndef __cplusplus

// Small offset between ray intersection and new ray origin to avoid self-intersections.
#define SMALL_OFFSET 0.0001

// For procedural intersections you must add custom hit kind.
#define RAY_KIND_PROCEDURAL_FRONT_FACE 1
#define RAY_KIND_PROCEDURAL_BACK_FACE  2

#endif