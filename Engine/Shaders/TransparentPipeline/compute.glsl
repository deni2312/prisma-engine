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

layout(std430, binding = 1) buffer Matrices
{
    mat4 modelMatrices[];
};

layout(std430, binding = 20) buffer MatricesCopy
{
    mat4 modelMatricesCopy[];
};

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 3) uniform FragmentData
{
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

layout(std430, binding = 0) buffer Material
{
    MaterialData materialData[];
};

layout(std430, binding = 21) buffer MaterialCopy
{
    MaterialData materialDataCopy[];
};

layout(std430, binding = 23) buffer IndicesData
{
    ivec4 indicesData[];
};

layout(std430, binding = 24) buffer Status
{
    uint status[];
};

layout(std430, binding = 25) buffer StatusCopy
{
    uint statusCopy[];
};

// Function to calculate depth from the camera for sorting
float calculateDepth(mat4 modelMatrix) {
    vec3 worldPosition = vec3(modelMatrix[3]); // Get position from model matrix
    vec3 viewPosition = vec3(viewPos) - worldPosition; // Transform to view space
    return length(viewPosition); // Depth value (negative for view direction)
}

uniform int size;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index == 0) {
        // Initialize indices
        for (int i = 0; i < size; i++) {
            indicesData[i].x = i;
        }

        // Count transparent materials in indicesData and create separate index lists
        int transparentCount = 0;
        for (int i = 0; i < size; i++) {
            if (materialDataCopy[indicesData[i].x].transparent) {
                int temp = indicesData[i].x;
                indicesData[i].x = indicesData[size - 1 - transparentCount].x;
                indicesData[size - 1 - transparentCount].x = temp;
                transparentCount++;
            }
        }

        int transparentStart = size - transparentCount;

        // Perform bubble sort on indices based on depth from copy buffers
        for (uint i = transparentStart; i < size - 1; i++) {
            for (uint j = transparentStart; j < size - 1 - (i- transparentStart); j++) {
                // Calculate depths using the copy buffers
                float depthA = calculateDepth(modelMatricesCopy[indicesData[j].x]);
                float depthB = calculateDepth(modelMatricesCopy[indicesData[j + 1].x]);

                // Sort in descending order (farthest first) for transparency blending
                if (depthA < depthB) {
                    // Swap indices instead of modifying the copy buffer data
                    int temp = indicesData[j].x;
                    indicesData[j].x = indicesData[j + 1].x;
                    indicesData[j + 1].x = temp;
                }
            }
        }
    }
}
