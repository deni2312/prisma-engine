#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct InstanceData {
    unsigned int  count;
    unsigned int  instanceCount;
    unsigned int  firstIndex;
    unsigned int  baseVertex;
    unsigned int  baseInstance;
};

layout(std430, binding = 18) buffer DrawElementsIndirectMesh
{
    InstanceData instanceData[];
};

layout(std430, binding = 1) readonly buffer Matrices
{
    mat4 modelMatrices[];
};

layout(std430, binding = 19) buffer DrawElementsIndirectMeshAnimate
{
    InstanceData instanceDataAnimation[];
};

layout(std430, binding = 6) readonly buffer AnimationMatrices
{
    mat4 modelAnimationMatrices[];
};

void main()
{

}
