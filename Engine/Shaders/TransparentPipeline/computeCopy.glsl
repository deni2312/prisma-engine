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

struct Plane {
    vec3 normal;
    float distance;
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

Frustum currentFrustum;


bool isInFrustum(uint index) {

    vec3 globalScale = vec3(length(modelMatricesCopy[index][0]), length(modelMatricesCopy[index][1]), length(modelMatricesCopy[index][2]));
    vec3 globalCenter= vec3(modelMatricesCopy[index] * vec4(aabbData[index].center.xyz, 1.f));
    float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);
    float radius = max(max(aabbData[index].center.x, aabbData[index].center.y), aabbData[index].center.z)*(maxScale*0.5);
    Plane[6] planes = Plane[6](
        currentFrustum.nearFace,
        currentFrustum.farFace,
        currentFrustum.rightFace,
        currentFrustum.leftFace,
        currentFrustum.topFace,
        currentFrustum.bottomFace
        );

    for (int i = 0; i < 6; i++) {
        if (dot(planes[i].normal, globalCenter) + planes[i].distance < -radius) {
            return false;
        }
    }

    return true;
}


// Function to construct the frustum planes
Frustum createFrustum() {
    Frustum frustum;

    mat4 viewData = inverse(view);

    vec3 camFront = vec3(normalize(viewData[2]));
    vec3 camUp = vec3(normalize(viewData[1]));
    vec3 camRight = vec3(normalize(viewData[0]));
    float halfVSide = zFar * tan(radians(fovY) * 0.5);
    float halfHSide = halfVSide * aspect;
    vec3 frontMultFar = camFront * zFar;

    vec3 camPosition = vec3(viewPos);

    // Near face
    frustum.nearFace.normal = camFront;
    frustum.nearFace.distance = dot(camPosition + camFront * zNear, camFront);

    // Far face
    frustum.farFace.normal = -camFront;
    frustum.farFace.distance = dot(camPosition + frontMultFar, -camFront);

    // Right face
    frustum.rightFace.normal = normalize(cross(camUp, frontMultFar - camRight * halfHSide));
    frustum.rightFace.distance = dot(camPosition, frustum.rightFace.normal);

    // Left face
    frustum.leftFace.normal = normalize(cross(frontMultFar + camRight * halfHSide, camUp));
    frustum.leftFace.distance = dot(camPosition, frustum.leftFace.normal);

    // Top face
    frustum.topFace.normal = normalize(cross(camRight, frontMultFar - camUp * halfVSide));
    frustum.topFace.distance = dot(camPosition, frustum.topFace.normal);

    // Bottom face
    frustum.bottomFace.normal = normalize(cross(frontMultFar + camUp * halfVSide, camRight));
    frustum.bottomFace.distance = dot(camPosition, frustum.bottomFace.normal);

    return frustum;
}

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
        currentFrustum = createFrustum();
        /*if (!isInFrustum(sortedIndex)) {
            status[index] = 0;
            instanceData[index].instanceCount = 0;
        }*/
    }
}