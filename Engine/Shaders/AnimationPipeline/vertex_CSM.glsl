#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

layout(std430, binding = 6) readonly buffer AnimationMatrices
{
    mat4 modelAnimationMatrices[];
};

struct SSBOAnimation {
    mat4 animations[MAX_BONES];
};

layout(std430, binding = 8) readonly buffer BoneMatrices
{
    SSBOAnimation boneMatrices[];
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
        vec4 localPosition = boneMatrices[gl_DrawID].animations[boneIds[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * weights[i];
    }
    gl_Position = modelAnimationMatrices[gl_DrawID] * totalPosition;
}