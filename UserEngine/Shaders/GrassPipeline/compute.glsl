#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Input: Grass positions (world space)
layout(std430, binding = 15) readonly buffer GrassPositions
{
    mat4 grassPositions[];  // Positions of grass instances in world space
};

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    ivec4 size;              // Size of the culled instances (size.x will store the count)
    mat4 grassCull[];       // Positions of culled instances
};

// Uniforms: View and Projection matrices
layout(std140, binding = 1) uniform MeshData
{
    mat4 view;              // View matrix
    mat4 projection;        // Projection matrix
};

void main()
{
    // Get the current instance index
    uint idx = gl_GlobalInvocationID.x;
    // Ensure we don't exceed the bounds of the buffer
    if (idx >= grassPositions.length())
        return;
    
    // Transform the grass position from world space to clip space
    mat4 worldPos = grassPositions[idx];
    vec4 grassCenter = worldPos[3];  // Get the center of the grass instance

    // Convert to clip space (projection * view * world)
    vec4 clipSpacePos = projection * view * grassCenter;

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