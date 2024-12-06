#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct InstanceData {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

layout(std430, binding = 18) buffer DrawElementsIndirectMesh {
    InstanceData instanceData[];
};

layout(std430, binding = 22) buffer DrawElementsIndirectMeshCopy {
    InstanceData instanceDataCopy[];
};

layout(std430, binding = 1) buffer Matrices {
    mat4 modelMatrices[];
};

layout(std430, binding = 20) buffer MatricesCopy {
    mat4 modelMatricesCopy[];
};

layout(std140, binding = 1) uniform MeshData {
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 4) uniform CameraData {
    float zNear;
    float zFar;
    float fovY;
    float aspect;
};

layout(std140, binding = 3) uniform FragmentData {
    vec4 viewPos;
    vec2 irradianceMap;
    vec2 prefilterMap;
    vec2 brdfLUT;
    vec2 paddingFragment;
};

struct MaterialData {
    vec2 diffuse;
    vec2 normal;
    vec2 roughness_metalness;
    vec2 specularMap;
    vec2 ambient_occlusion;
    bool transparent;
    float padding;
};

layout(std430, binding = 0) buffer Material {
    MaterialData materialData[];
};

layout(std430, binding = 21) buffer MaterialCopy {
    MaterialData materialDataCopy[];
};

layout(std430, binding = 23) buffer IndicesData {
    ivec4 indicesData[];
};

layout(std430, binding = 24) buffer Status {
    uint status[];
};

layout(std430, binding = 25) buffer StatusCopy {
    uint statusCopy[];
};

struct AABB {
    vec4 center;  // xyz: center position, w: padding
    vec4 extents; // xyz: extents (half-size), w: padding
};

layout(std430, binding = 27) buffer AABBData {
    AABB aabbData[];
};


uniform bool initIndices = false;

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (initIndices) {
        indicesData[index].x = int(index);
    }
    else {
        int sortedIndex = indicesData[index].x;

        // Copy data
        instanceData[index] = instanceDataCopy[sortedIndex];
        modelMatrices[index] = modelMatricesCopy[sortedIndex];
        materialData[index] = materialDataCopy[sortedIndex];
        status[index] = statusCopy[sortedIndex];
        
    }
}