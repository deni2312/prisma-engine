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

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index == 0) {
        // Simple in-place bubble sort for demonstration (serial, single invocation)
        uint n = instanceData.length();
        for (uint i = 0; i < n - 1; i++) {
            for (uint j = 0; j < n - 1; j++) {
                // Calculate depths for two consecutive instances
                float depthA = calculateDepth(modelMatrices[i]);
                float depthB = calculateDepth(modelMatrices[i + 1]);

                // Sort in descending order (farthest first) for correct transparency blending
                if (depthA < depthB) {
                    // Swap instance data in-place
                    InstanceData temp = instanceData[i];
                    instanceData[i] = instanceData[i + 1];
                    instanceData[i + 1] = temp;

                    // Swap model matrices in-place to keep data in sync
                    mat4 tempMatrix = modelMatrices[i];
                    modelMatrices[i] = modelMatrices[i + 1];
                    modelMatrices[i + 1] = tempMatrix;

                    // Swap texture data in-place
                    MaterialData tempMaterial = materialData[i];
                    materialData[i] = materialData[i + 1];
                    materialData[i + 1] = tempMaterial;
                }
            }
        }
    }
}
