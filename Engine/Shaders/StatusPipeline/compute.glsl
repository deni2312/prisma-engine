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

layout(std430, binding = 25) buffer StatusCopy
{
    uint statusCopy[];
};

layout(std430, binding = 19) buffer DrawElementsIndirectMeshAnimation
{
    InstanceData instanceDataAnimation[];
};

layout(std430, binding = 26) buffer StatusAnimation
{
    uint statusAnimation[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index == 0) {
        for (int i = 0; i < statusCopy.length(); i++) {
            instanceDataCopy[i].instanceCount = statusCopy[i];
        }
        for (int i = 0; i < statusAnimation.length(); i++) {
            instanceDataAnimation[i].instanceCount = statusAnimation[i];
        }
    }
}
