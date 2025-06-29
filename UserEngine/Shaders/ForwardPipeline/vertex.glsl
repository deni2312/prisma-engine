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
//layout(location = 4) out mat3 outTBN;

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
    int currentId=gl_InstanceIndex;

    mat4 modelMatrix = modelsData[currentId].model;
    mat4 normalMatrix = modelsData[currentId].normal;

    vec4 worldPos = modelMatrix * vec4(inPos, 1.0);
    vec3 worldNormal = normalize(vec3(normalMatrix * vec4(inNormal, 0.0)));
    //vec3 worldTangent = normalize(vec3(normalMatrix * vec4(inTangent, 0.0)));
    //vec3 worldBitangent = normalize(vec3(normalMatrix * vec4(inBitangent, 0.0)));

    outUv = inUV;
    outFragPos = worldPos.xyz;
    outNormal = worldNormal;
    outDrawId = currentId;

    // Construct TBN matrix
    //outTBN = mat3(worldTangent, worldBitangent, worldNormal);

    gl_Position = projection * view * worldPos;
}