layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 1, rgba8) writeonly uniform image2D screenTexture;

void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    vec4 color = vec4(1.0, 0.0, 1.0, 1.0); // Solid magenta
    imageStore(screenTexture, gid, color);
}
