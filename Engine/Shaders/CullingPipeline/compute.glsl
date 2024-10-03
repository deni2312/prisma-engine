#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

struct GrassPosition {
    mat4 direction;
    mat4 position;
};

layout(std430, binding = 1) readonly buffer Matrices
{
    mat4 modelMatrices[];
};

layout(std430, binding = 17) buffer DrawElementsIndirect
{
    unsigned int  count;
    unsigned int  instanceCount;
    unsigned int  firstIndex;
    unsigned int  baseVertex;
    unsigned int  baseInstance;
};

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    GrassPosition grassCull[];        // Positions of culled instances
};

// Uniforms: View and Projection matrices
layout(std140, binding = 1) uniform MeshData
{
    mat4 view;              // View matrix
    mat4 projection;        // Projection matrix
};

uniform mat4 model;
uniform mat4 currentProjection;

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

}