layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 1, rgba8) writeonly uniform image2D screenTexture;

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

struct SizeMeshes
{
    uint vertexBase;
    uint indexBase;
    uint vertexSize;
    uint indexSize;
};

uniform TotalSizes
{
    ivec4 totalMeshes;
};

struct Vertex {
    vec4 vertex;
};

buffer vertices{
    Vertex vertices_data[];
};

buffer indices{
    uint indices_data[];
};

buffer SizeData{
    SizeMeshes size_data[];
};

struct MeshData
{
    mat4 model;
    mat4 normal;
};


readonly buffer models{
    MeshData modelsData[];
};


bool rayTriangleIntersect(
    vec3 orig, vec3 dir,
    vec3 v0, vec3 v1, vec3 v2,
    out float t)
{
    float EPSILON = 0.000001;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(dir, edge2);
    float a = dot(edge1, h);
    if (abs(a) < EPSILON) return false;

    float f = 1.0 / a;
    vec3 s = orig - v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    if (v < 0.0 || u + v > 1.0) return false;

    float tTemp = f * dot(edge2, q);
    if (tTemp > EPSILON) {
        t = tTemp;
        return true;
    }
    return false;
}

void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 screenSize = imageSize(screenTexture);

    // NDC coordinates [-1, 1]
    vec2 uv = (vec2(gid) + 0.5) / vec2(screenSize) * 2.0 - 1.0;

    // Invert projection to get ray direction in view space
    vec4 rayClip = vec4(uv, -1.0, 1.0); // z = -1 for near plane
    vec4 rayView = inverse(projection) * rayClip;
    rayView = vec4(rayView.xy, -1.0, 0.0); // set direction, w=0

    vec3 rayOrigin = viewPos.xyz;
    vec3 rayDir = normalize((inverse(view) * rayView).xyz);

    float closestT = 1e30;
    bool hit = false;

    for (int i = 0; i < totalMeshes.r; i++) {
        SizeMeshes currentSize = size_data[i];
        mat4 model = modelsData[i].model;

        for (uint tri = currentSize.indexBase; tri < currentSize.indexBase + currentSize.indexSize; tri += 3) {
            uint i0 = indices_data[tri + 0];
            uint i1 = indices_data[tri + 1];
            uint i2 = indices_data[tri + 2];

            vec3 v0 = (model * vertices_data[currentSize.vertexBase + i0].vertex).xyz;
            vec3 v1 = (model * vertices_data[currentSize.vertexBase + i1].vertex).xyz;
            vec3 v2 = (model * vertices_data[currentSize.vertexBase + i2].vertex).xyz;

            float t;
            if (rayTriangleIntersect(rayOrigin, rayDir, v0, v1, v2, t)) {
                if (t < closestT) {
                    closestT = t;
                    hit = true;
                }
            }
        }
    }

    if (hit) {
        imageStore(screenTexture, gid, vec4(1.0, 0.5, 0.0, 1.0)); // hit: orange
    } else {
        imageStore(screenTexture, gid, vec4(0.0, 0.0, 0.0, 1.0)); // no hit: black
    }
}
