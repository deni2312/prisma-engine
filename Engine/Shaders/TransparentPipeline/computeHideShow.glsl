#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct InstanceData {
    unsigned int  count;
    unsigned int  instanceCount;
    unsigned int  firstIndex;
    unsigned int  baseVertex;
    unsigned int  baseInstance;
};

layout(std430, binding = 22) buffer DrawElementsIndirectMeshCopy
{
    InstanceData instanceDataCopy[];
};

uniform unsigned int show = 1;

void main() {
    uint index = gl_GlobalInvocationID.x;
    // Sort using indices to avoid modifying the copy buffers
    instanceDataCopy[index].instanceCount = show;
}
