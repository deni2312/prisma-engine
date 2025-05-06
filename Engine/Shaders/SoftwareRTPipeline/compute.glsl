layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 1, rgba8) writeonly uniform image2D screenTexture;

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

uniform SizeData
{
    uint vertexSize;
    uint indexSize;
    ivec2 padding;
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

void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 screenSize = imageSize(screenTexture);

    for (uint tri = 0; tri + 2 < indexSize; tri += 3)
    {
        int i0 = indices_data[tri + 0].r;
        int i1 = indices_data[tri + 1].r;
        int i2 = indices_data[tri + 2].r;

        if (i0 < 0 || i1 < 0 || i2 < 0 || i0 >= int(vertexSize) || i1 >= int(vertexSize) || i2 >= int(vertexSize))
            continue;

        vec4 v0 = projection * view * vertices_data[i0].vertex;
        vec4 v1 = projection * view * vertices_data[i1].vertex;
        vec4 v2 = projection * view * vertices_data[i2].vertex;

        v0.xyz /= v0.w;
        v1.xyz /= v1.w;
        v2.xyz /= v2.w;

        ivec2 p0 = ivec2((v0.xy * 0.5 + 0.5) * vec2(screenSize));
        ivec2 p1 = ivec2((v1.xy * 0.5 + 0.5) * vec2(screenSize));
        ivec2 p2 = ivec2((v2.xy * 0.5 + 0.5) * vec2(screenSize));

        if (gid == p0 || gid == p1 || gid == p2) {
            imageStore(screenTexture, gid, vec4(1.0, 0.5, 0.0, 1.0)); // orange pixel
        }
    }
}
