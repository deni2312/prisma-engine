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

layout(std430, binding = 22) buffer DrawElementsIndirectMeshCopy
{
    InstanceData instanceDataCopy[];
};

struct MaterialData {
    vec2 diffuse;
    vec2 normal;
    vec2 roughness_metalness;
    vec2 specularMap;
    vec2 ambient_occlusion;
    int transparent;
    float padding;
    vec4 materialColor;
};

layout(std430, binding = 0) buffer Material
{
    MaterialData materialData[];
};


struct StatusData{
    uint status;
    int plainColor;
    vec2 padding;
};


layout(std430, binding = 24) buffer Status {
    StatusData status[];
};

uniform bool transparent;


layout(std430, binding = 29) buffer Ids {
    uint ids[];
};

layout(binding = 0) uniform atomic_uint counterSize;

void main() {
    uint index = gl_GlobalInvocationID.x;
    int size=int(atomicCounter(counterSize));
    if (status[index].status > 0 && index<size) {
        if (transparent) {
            if (materialData[ids[index]].transparent==1) {
                instanceData[index].instanceCount = 1;
            }
            else {
                instanceData[index].instanceCount = 0;
            }
        }
        else {
            if (materialData[ids[index]].transparent==0) {
                instanceData[index].instanceCount = 1;
            }
            else {
                instanceData[index].instanceCount = 0;
            }
        }
    }
}
