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
    ivec4 indices_data[];
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


void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 screenSize = imageSize(screenTexture);
    vec2 fragCoord = (vec2(gid) + 0.5) / vec2(screenSize) * 2.0 - 1.0;
    for(int i=0;i<totalMeshes.r;i++){
        SizeMeshes currentSize=size_data[i];
        for (uint tri = currentSize.indexBase; tri < currentSize.indexBase+currentSize.indexSize; tri += 3)
        {
            int i0 = indices_data[tri + 0].r;
            int i1 = indices_data[tri + 1].r;
            int i2 = indices_data[tri + 2].r;

            mat4 model=modelsData[i].model;
            vec4 v0 = projection * view * model*vertices_data[currentSize.vertexBase+i0].vertex;
            vec4 v1 = projection * view * model*vertices_data[currentSize.vertexBase+i1].vertex;
            vec4 v2 = projection * view * model*vertices_data[currentSize.vertexBase+i2].vertex;

            if (v0.w <= 0.0 || v1.w <= 0.0 || v2.w <= 0.0)
                continue;

            v0.xyz /= v0.w;
            v1.xyz /= v1.w;
            v2.xyz /= v2.w;

            vec3 ndc0 = v0.xyz;
            vec3 ndc1 = v1.xyz;
            vec3 ndc2 = v2.xyz;

            if ((ndc0.x < -1.0 && ndc1.x < -1.0 && ndc2.x < -1.0) ||
                (ndc0.x >  1.0 && ndc1.x >  1.0 && ndc2.x >  1.0) ||
                (ndc0.y < -1.0 && ndc1.y < -1.0 && ndc2.y < -1.0) ||
                (ndc0.y >  1.0 && ndc1.y >  1.0 && ndc2.y >  1.0))
                continue;

            vec2 a = v0.xy;
            vec2 b = v1.xy;
            vec2 c = v2.xy;

            // Compute barycentric coordinates
            float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            float w0 = (b.x - fragCoord.x) * (c.y - fragCoord.y) - (b.y - fragCoord.y) * (c.x - fragCoord.x);
            float w1 = (c.x - fragCoord.x) * (a.y - fragCoord.y) - (c.y - fragCoord.y) * (a.x - fragCoord.x);
            float w2 = (a.x - fragCoord.x) * (b.y - fragCoord.y) - (a.y - fragCoord.y) * (b.x - fragCoord.x);

            if ((w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0) || 
                (w0 <= 0.0 && w1 <= 0.0 && w2 <= 0.0)) {
                imageStore(screenTexture, gid, vec4(1.0, 0.5, 0.0, 1.0)); // orange fill
            }
        }
    }
}
