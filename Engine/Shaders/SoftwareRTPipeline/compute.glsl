layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 1, rgba8) writeonly uniform image2D screenTexture;

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
    vec4 color = vec4(1.0, vertices_data[0].vertex.r, indices_data[0].r, 1.0); // Solid magenta
    imageStore(screenTexture, gid, color);
}
