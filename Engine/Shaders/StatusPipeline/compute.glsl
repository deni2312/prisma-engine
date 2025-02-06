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

struct StatusData{
    uint status;
    int plainColor;
    vec2 padding;
};


layout(std430, binding = 25) buffer StatusCopy {
    StatusData statusCopy[];
};

layout(std430, binding = 19) buffer DrawElementsIndirectMeshAnimation
{
    InstanceData instanceDataAnimation[];
};

layout(std430, binding = 26) buffer StatusAnimation
{
    StatusData statusAnimation[];
};

uniform int size;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index < size) {
        instanceDataCopy[index].instanceCount = statusCopy[index].status;
    }
    else {
        instanceDataAnimation[index-size].instanceCount = statusAnimation[index-size].status;
    }
}
