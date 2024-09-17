#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Input: Grass positions (world space)
layout(std430, binding = 15) buffer GrassPositions
{
    vec4 grassPositions[];  // Positions of grass instances in world space
};

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    ivec4 size;              // Size of the culled instances (size.x will store the count)
    vec4 grassCull[];        // Positions of culled instances
};

// Uniforms: View and Projection matrices
layout(std140, binding = 1) uniform MeshData
{
    mat4 view;              // View matrix
    mat4 projection;        // Projection matrix
};

uniform mat4 model;

void main()
{
    // Calculate the 2D global invocation index based on both x and y
    // Assuming you process in a grid-like fashion (e.g., width x height)
    uint workGroupWidth = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    uint idx = gl_GlobalInvocationID.y * workGroupWidth + gl_GlobalInvocationID.x;

    // Ensure we don't exceed the bounds of the buffer
    if (idx >= grassPositions.length())
        return;

    // Transform the grass position from world space to clip space
    vec4 worldPos = grassPositions[idx];

    // Convert to clip space (projection * view * world)
    vec4 clipSpacePos = projection * view * model * worldPos;

    // Perform frustum culling: clip-space coordinates must be inside [-w, w] range
    if (clipSpacePos.x > -clipSpacePos.w && clipSpacePos.x < clipSpacePos.w &&
        clipSpacePos.y > -clipSpacePos.w && clipSpacePos.y < clipSpacePos.w &&
        clipSpacePos.z > -clipSpacePos.w && clipSpacePos.z < clipSpacePos.w)
    {
        // Atomically increment the counter for culled instances and get the index
        uint culledIdx = atomicAdd(size.x, 1);

        // Store the culled instance in the culled buffer
        grassCull[culledIdx] = worldPos;
    }
}
