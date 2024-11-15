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

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index == 0) {
        // Sort using indices to avoid modifying the copy buffers
        uint size = statusCopy.length();
        // Initialize indices
        for (int i = 0; i < size; i++) {
            indicesData[i].x = i;
        }

        uint bitMask = 1u;
        int maxBits = 32; // Assuming 32-bit floats represented as integers

        // Perform sorting bit-by-bit
        for (int bit = 0; bit < maxBits; bit++) {
            int zeroCount = 0;

            // Create a count of zeros for the current bit position
            for (int i = 0; i < size; i++) {
                float depth = calculateDepth(modelMatricesCopy[indicesData[i].x]);
                int intDepth = floatBitsToInt(depth);
                if ((intDepth & bitMask) == 0u) {
                    zeroCount++;
                }
            }

            // Compute the output indices
            int zeroIndex = 0;
            int oneIndex = zeroCount;
            for (int i = 0; i < size; i++) {
                float depth = calculateDepth(modelMatricesCopy[indicesData[i].x]);
                int intDepth = floatBitsToInt(depth);
                if ((intDepth & bitMask) == 0u) {
                    indicesData[zeroIndex++].y = indicesData[i].x;
                }
                else {
                    indicesData[oneIndex++].y = indicesData[i].x;
                }
            }

            // Copy sorted data back to indicesData
            for (int i = 0; i < size; i++) {
                indicesData[i].x = indicesData[i].y;
            }

            // Move to the next bit
            bitMask <<= 1u;
        }

        // Reverse the array to ensure descending order (largest first)
        for (int i = 0; i < size / 2; i++) {
            int temp = indicesData[i].x;
            indicesData[i].x = indicesData[size - 1 - i].x;
            indicesData[size - 1 - i].x = temp;
        }

        // Write sorted data from copy buffers to main buffers
        for (uint i = 0; i < size; i++) {
            int sortedIndex = indicesData[i].x;
            instanceData[i] = instanceDataCopy[sortedIndex];
            modelMatrices[i] = modelMatricesCopy[sortedIndex];
            materialData[i] = materialDataCopy[sortedIndex];
            status[i] = statusCopy[sortedIndex];
        }
    }
}
