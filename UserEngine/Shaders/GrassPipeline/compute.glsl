#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Input: Grass positions (world space)
layout(std430, binding = 15) buffer GrassPositions
{
    mat4 grassPositions[];  // Positions of grass instances in world space
};

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    mat4 grassCull[];        // Positions of culled instances
};

layout(std430, binding = 17) buffer DrawElementsIndirect
{
    unsigned int  count;
    unsigned int  instanceCount;
    unsigned int  firstIndex;
    unsigned int  baseVertex;
    unsigned int  baseInstance;
    unsigned int padding;
    unsigned int padding1;
    unsigned int padding2;
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


    if (idx == 0) {
        instanceCount = 0;
    }
    barrier();// Wait till all threads reach this point
    // Ensure we don't exceed the bounds of the buffer
    if (idx >= grassPositions.length())
        return;

    // Transform the grass position from world space to clip space
    vec4 worldPos = vec4(grassPositions[idx][3].xyz, 1.0);

    // Convert to clip space (projection * view * world)
    vec4 clipSpacePos = projection * view * model * worldPos;

    // Perform frustum culling: clip-space coordinates must be inside [-w, w] range
    if (clipSpacePos.x > -clipSpacePos.w && clipSpacePos.x < clipSpacePos.w &&
        clipSpacePos.y > -clipSpacePos.w && clipSpacePos.y < clipSpacePos.w &&
        clipSpacePos.z > -clipSpacePos.w && clipSpacePos.z < clipSpacePos.w)
    {
        // Atomically increment the counter for culled instances and get the index
        uint culledIdx = atomicAdd(instanceCount, 1);

        // Store the culled instance in the culled buffer
        grassCull[culledIdx] = grassPositions[idx];
    }
}