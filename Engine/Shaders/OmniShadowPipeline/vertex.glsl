#extension GL_ARB_shader_draw_parameters : enable

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

layout(location = 0) in vec3 inPos;
#ifdef ANIMATION
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;
#endif

struct MeshData
{
    mat4 model;
    mat4 normal;
};

readonly buffer models{
    MeshData modelsData[];
};

#ifdef ANIMATION
  
struct SSBOAnimation {
    mat4 animations[MAX_BONES];
};

readonly buffer animations{
    SSBOAnimation modelAnimations[];
};

#endif


void main()
{
#ifdef ANIMATION
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

    gl_Position = modelsData[gl_DrawIDARB].model * vec4(totalPosition.xyz, 1.0);
#else
    gl_Position = modelsData[gl_DrawIDARB].model * vec4(inPos, 1.0);
#endif
}