#extension GL_ARB_shader_draw_parameters : enable

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
layout(location = 4) in vec3 inBiTangent;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec3 outNormal;
layout(location = 3) flat out int outDrawId;

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
    vec4 worldPos = modelsData[gl_DrawIDARB].model * vec4(inPos, 1.0);
    outNormal = vec3(modelsData[gl_DrawIDARB].normal * vec4(inNormal, 1.0));
    gl_Position = projection*view*worldPos;
    outUv = inUV;
    outFragPos = worldPos.xyz; // Store world position for fragment shading
    outDrawId = gl_DrawIDARB;
}