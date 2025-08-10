#extension GL_ARB_shader_draw_parameters : enable

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
#ifdef ANIMATION
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;
#endif

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec3 outNormal;
layout(location = 3) flat out int outDrawId;
layout(location = 4) out mat3 outTBN;

struct MeshData
{
    mat4 model;
    mat4 normal;
};

#ifdef ANIMATION
  
struct SSBOAnimation {
    mat4 animations[MAX_BONES];
};

readonly buffer animations{
    SSBOAnimation modelAnimations[];
};

#endif

readonly buffer models{
    MeshData modelsData[];
};

#ifndef ANIMATION
readonly buffer opaqueIndices{
    int opaqueIndicesData[];
};
#endif

void main()
{
#ifndef ANIMATION
int currentId=opaqueIndicesData[gl_DrawIDARB];
#else
int currentId=gl_DrawIDARB;
#endif


#ifdef ANIMATION
    vec4 totalPosition = vec4(0.0f);
    vec3 localNormal = vec3(0.0f);
    vec3 localTangent = vec3(0.0f);
    vec3 localBitangent = vec3(0.0f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (boneIds[i] == -1)
            continue;
        if (boneIds[i] >= MAX_BONES)
        {
            totalPosition = vec4(inPos, 1.0f);
            break;
        }

        mat4 boneTransform = modelAnimations[currentId].animations[boneIds[i]];
        mat3 boneRotation = mat3(boneTransform); // Extract rotation/scale part

        totalPosition += boneTransform * vec4(inPos, 1.0) * weights[i];
        localNormal += boneRotation * inNormal * weights[i];
        localTangent += boneRotation * inTangent * weights[i];
        localBitangent += boneRotation * inBitangent * weights[i];
    }

    mat4 modelMatrix = modelsData[currentId].model;
    mat4 normalMatrix = modelsData[currentId].normal;
    vec4 worldPos = modelMatrix * vec4(totalPosition.xyz, 1.0);
    vec3 worldNormal = normalize(vec3(normalMatrix * vec4(localNormal, 0.0)));
    vec3 worldTangent = normalize(vec3(normalMatrix * vec4(localTangent, 0.0)));
    vec3 worldBitangent = normalize(vec3(normalMatrix * vec4(localBitangent, 0.0)));
    // Construct TBN matrix
    outTBN = mat3(worldTangent, worldBitangent, worldNormal);

    outUv = inUV;
    outFragPos = worldPos.xyz;
    outNormal = worldNormal;
    outDrawId = currentId;
    gl_Position = projection * view * worldPos;
#else
    mat4 modelMatrix = modelsData[currentId].model;
    mat4 normalMatrix = modelsData[currentId].normal;

    vec4 worldPos = modelMatrix * vec4(inPos, 1.0);
    vec3 worldNormal = normalize(vec3(normalMatrix * vec4(inNormal, 0.0)));
    vec3 worldTangent = normalize(vec3(normalMatrix * vec4(inTangent, 0.0)));
    vec3 worldBitangent = normalize(vec3(normalMatrix * vec4(inBitangent, 0.0)));

    outUv = inUV;
    outFragPos = worldPos.xyz;
    outNormal = worldNormal;
    outDrawId = currentId;

    // Construct TBN matrix
    outTBN = mat3(worldTangent, worldBitangent, worldNormal);

    gl_Position = projection * view * worldPos;
#endif
}