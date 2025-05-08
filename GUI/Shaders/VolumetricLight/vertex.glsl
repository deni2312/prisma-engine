#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

struct MeshData
{
    mat4 model;
    mat4 normal;
};

readonly buffer models{
    MeshData modelsData[];
};

void main()
{
    mat4 modelMatrix = modelsData[gl_DrawIDARB].model;

    vec4 worldPos = modelMatrix * vec4(inPos, 1.0);

    gl_Position = projection * view * worldPos;
}