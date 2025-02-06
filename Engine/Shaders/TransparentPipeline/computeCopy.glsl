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

layout(std140, binding = 1) uniform MeshData {
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 28) buffer CameraData {
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
    vec4 materialColor;
};

layout(std430, binding = 0) buffer Material {
    MaterialData materialData[];
};

struct StatusData{
    uint status;
    bool plainColor;
    vec2 padding;
};


layout(std430, binding = 24) buffer Status {
    StatusData status[];
};

layout(std430, binding = 25) buffer StatusCopy {
    StatusData statusCopy[];
};

struct AABB {
    vec4 center;  // xyz: center position, w: padding
    vec4 extents; // xyz: extents (half-size), w: padding
};

layout(std430, binding = 27) buffer AABBData {
    AABB aabbData[];
};


layout(std430, binding = 29) buffer Ids {
    uint ids[];
};


// Function to transform AABB from local space to world space
AABB transformAABB(AABB localAABB, mat4 modelMatrix) {
    vec3 worldCenter = vec3(modelMatrix * localAABB.center);

    // Compute the transformed extents
    vec3 axisX = vec3(modelMatrix[0][0], modelMatrix[1][0], modelMatrix[2][0]);
    vec3 axisY = vec3(modelMatrix[0][1], modelMatrix[1][1], modelMatrix[2][1]);
    vec3 axisZ = vec3(modelMatrix[0][2], modelMatrix[1][2], modelMatrix[2][2]);

    vec3 worldExtents = abs(localAABB.extents.xyz.x * axisX) +
        abs(localAABB.extents.xyz.y * axisY) +
        abs(localAABB.extents.xyz.z * axisZ);

    return AABB(vec4(worldCenter, 1.0), vec4(worldExtents, 0.0));
}

// Function to check if the transformed AABB is in the frustum
bool isAABBInFrustum(mat4 viewProjection, AABB worldAABB) {
    vec3 absExtents = worldAABB.extents.xyz;

    // Loop through all 6 planes
    for (int i = 0; i < 6; ++i) {
        vec3 planeNormal;
        float planeOffset;

        if (i == 0) { // Left plane
            planeNormal = vec3(viewProjection[0][3] + viewProjection[0][0],
                viewProjection[1][3] + viewProjection[1][0],
                viewProjection[2][3] + viewProjection[2][0]);
            planeOffset = viewProjection[3][3] + viewProjection[3][0];
        }
        else if (i == 1) { // Right plane
            planeNormal = vec3(viewProjection[0][3] - viewProjection[0][0],
                viewProjection[1][3] - viewProjection[1][0],
                viewProjection[2][3] - viewProjection[2][0]);
            planeOffset = viewProjection[3][3] - viewProjection[3][0];
        }
        else if (i == 2) { // Bottom plane
            planeNormal = vec3(viewProjection[0][3] + viewProjection[0][1],
                viewProjection[1][3] + viewProjection[1][1],
                viewProjection[2][3] + viewProjection[2][1]);
            planeOffset = viewProjection[3][3] + viewProjection[3][1];
        }
        else if (i == 3) { // Top plane
            planeNormal = vec3(viewProjection[0][3] - viewProjection[0][1],
                viewProjection[1][3] - viewProjection[1][1],
                viewProjection[2][3] - viewProjection[2][1]);
            planeOffset = viewProjection[3][3] - viewProjection[3][1];
        }
        else if (i == 4) { // Near plane
            planeNormal = vec3(viewProjection[0][3] + viewProjection[0][2],
                viewProjection[1][3] + viewProjection[1][2],
                viewProjection[2][3] + viewProjection[2][2]);
            planeOffset = viewProjection[3][3] + viewProjection[3][2];
        }
        else if (i == 5) { // Far plane
            planeNormal = vec3(viewProjection[0][3] - viewProjection[0][2],
                viewProjection[1][3] - viewProjection[1][2],
                viewProjection[2][3] - viewProjection[2][2]);
            planeOffset = viewProjection[3][3] - viewProjection[3][2];
        }

        // Normalize the plane
        float length = length(planeNormal);
        planeNormal /= length;
        planeOffset /= length;

        // Compute the distance from the AABB center to the plane
        float distance = dot(worldAABB.center.xyz, planeNormal) + planeOffset;

        // Compute the projected radius of the AABB onto the plane
        float projectedRadius = dot(absExtents, abs(planeNormal));

        // If the AABB is completely outside this plane, cull it
        if (distance + projectedRadius < 0.0) {
            return false;
        }
    }
    return true;
}

uniform int initIndices = 0;

layout(binding = 0) uniform atomic_uint counterSize;

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (initIndices==0) {
        atomicCounterExchange(counterSize, 0);
    }
    if(initIndices==1) {


        // Compute the view-projection matrix
        mat4 viewProjection = projection * view;

        // Transform the AABB to world space
        AABB worldAABB = transformAABB(aabbData[index], modelMatrices[index]);

        // Perform frustum culling
        if (isAABBInFrustum(viewProjection, worldAABB)) {
            uint culledIdx = atomicCounterIncrement(counterSize);
            // Copy data
            ids[culledIdx] = index;
        }
    }
    if(initIndices==2)
    {
        int size=int(atomicCounter(counterSize));
        if(index<size){
            instanceData[index] = instanceDataCopy[ids[index]];
            status[index] = statusCopy[ids[index]];
        }
    }
}