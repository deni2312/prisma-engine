#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec2 aBitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 FragPos;

out vec2 TexCoords;

out vec3 Normal;
out vec4 shadowDirData[16];

flat out int drawId;

uniform mat4 lightSpaceMatrix;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 6) buffer AnimationMatrices
{
    mat4 modelAnimationMatrices[];
};

struct ShadowData {
    mat4 shadow;
};

layout(std430, binding = 4) buffer ShadowMatrices
{
    vec4 lenMat;
    ShadowData shadowMatrices[];
};

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (boneIds[i] == -1)
            continue;
        if (boneIds[i] >= MAX_BONES)
        {
            totalPosition = vec4(aPos, 1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * weights[i];
    }

    drawId = gl_DrawID;
    FragPos = vec3(modelAnimationMatrices[gl_DrawID] * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    mat3 normalMatrix = mat3(transpose(inverse(mat3(modelAnimationMatrices[gl_DrawID]))));
    Normal = normalMatrix * aNormal;
    for (int i = 0; i < lenMat.r; i++) {
        shadowDirData[i] = shadowMatrices[i].shadow * vec4(FragPos, 1.0);
    }

    gl_Position = projection * view * totalPosition;
}