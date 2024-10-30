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
};

layout(std430, binding = 0) buffer Material
{
    MaterialData materialData[];
};

layout(std430, binding = 24) buffer Status
{
    uint status[];
};

uniform bool transparent;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index == 0) {
        for (int i = 0; i < materialData.length(); i++) {
            if (status[i] > 0) {
                if (transparent) {
                    if (materialData[i].transparent==1) {
                        instanceData[i].instanceCount = 1;
                    }
                    else {
                        instanceData[i].instanceCount = 0;
                    }
                }
                else {
                    if (materialData[i].transparent==0) {
                        instanceData[i].instanceCount = 1;
                    }
                    else {
                        instanceData[i].instanceCount = 0;
                    }
                }
            }
        }
    }
}
