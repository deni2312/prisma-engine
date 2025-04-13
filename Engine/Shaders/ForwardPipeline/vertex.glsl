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
layout(location = 4) in vec3 inBitangent;

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

readonly buffer models{
    MeshData modelsData[];
};

void main()
{
    mat4 modelMatrix = modelsData[gl_DrawIDARB].model;
    mat4 normalMatrix = modelsData[gl_DrawIDARB].normal;

    vec4 worldPos = modelMatrix * vec4(inPos, 1.0);
    vec3 worldNormal = normalize(vec3(normalMatrix * vec4(inNormal, 0.0)));
    vec3 worldTangent = normalize(vec3(normalMatrix * vec4(inTangent, 0.0)));
    vec3 worldBitangent = normalize(vec3(normalMatrix * vec4(inBitangent, 0.0)));

    outUv = inUV;
    outFragPos = worldPos.xyz;
    outNormal = worldNormal;
    outDrawId = gl_DrawIDARB;

    // Construct TBN matrix
    outTBN = mat3(worldTangent, worldBitangent, worldNormal);

    gl_Position = projection * view * worldPos;
}