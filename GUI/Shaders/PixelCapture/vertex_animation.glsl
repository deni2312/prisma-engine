#extension GL_ARB_shader_draw_parameters : enable

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;


layout(location = 0) in vec3 inPos;

layout(location = 0) flat out int outDrawId;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;


struct MeshData
{
    mat4 model;
    mat4 normal;
};


uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

readonly buffer models{
    MeshData modelsData[];
};

struct SSBOAnimation {
    mat4 animations[MAX_BONES];
};

readonly buffer animations{
    SSBOAnimation modelAnimations[];
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
            totalPosition = vec4(inPos, 1.0f);
            break;
        }
        vec4 localPosition = modelAnimations[gl_DrawIDARB].animations[boneIds[i]] * vec4(inPos, 1.0f);
        totalPosition += localPosition * weights[i];
    }

    gl_Position =projection * view * modelsData[gl_DrawIDARB].model * vec4(totalPosition.xyz, 1.0);
    outDrawId = gl_DrawIDARB;
}