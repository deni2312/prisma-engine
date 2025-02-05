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
    vec2 textureLut;
    vec2 textureM;
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


// Function to calculate depth from the camera for sorting
float calculateDepth(mat4 modelMatrix) {
    vec3 worldPosition = vec3(modelMatrix[3]); // Get position from model matrix
    vec3 viewPosition = vec3(viewPos) - worldPosition; // Transform to view space
    return length(viewPosition); // Depth value (negative for view direction)
}


layout(binding = 0) uniform atomic_uint counterSize;

layout(std430, binding = 29) buffer Ids {
    uint ids[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;
    int size=int(atomicCounter(counterSize));

    if (index == 0 && size>0) {
        int transparentCount=0;
        for (uint i = 0; i < size; i++) {
            if (materialData[ids[i]].transparent) { // Check if the mesh is transparent
                // Swap the transparent mesh to the front of the array
                if (i != transparentCount) {
                    uint temp = ids[transparentCount];
                    ids[transparentCount] = ids[i];
                    ids[i] = temp;
                }
                transparentCount++;
            }
        }
        // Perform bubble sort on the transparent portion of the `ids` array
        for (int i = 0; i < transparentCount - 1; i++) {
            for (int j = 0; j < transparentCount - 1 - i; j++) {
                // Calculate depths using the copy buffers
                float depthA = calculateDepth(modelMatrices[ids[j]]);
                float depthB = calculateDepth(modelMatrices[ids[j + 1]]);

                // Sort in descending order (farthest first) for transparency blending
                if (depthA < depthB) {
                    // Swap indices
                    uint temp = ids[j];
                    ids[j] = ids[j + 1];
                    ids[j + 1] = temp;
                }
            }
        }
    }
}
