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
//layout(location = 4) out mat3 outTBN;

struct MeshData
{
    mat4 model;
    mat4 normal;
};

readonly buffer models{
    MeshData modelsData[];
};

//time.r contains time
uniform TimeBuffer{
    vec4 time;
};

void main()
{
    int currentId = gl_InstanceIndex;

    mat4 modelMatrix = modelsData[currentId].model;
    mat4 normalMatrix = modelsData[currentId].normal;

    float swayStrength = 0.1;   // max displacement at the top
    float frequency = 2.0;
    float speed = 1.5;

    // sway is strongest at the top (inPos.y), zero at base
    float swayFactor = clamp(inPos.y, 0.0, 1.0); // assumes grass height is ~1 unit tall

    // optional per-instance offset for variation
    float instanceOffset = float(currentId) * 12.9898;
    float phase = sin(dot(vec2(currentId, currentId), vec2(12.9898, 78.233))) * 43758.5453;
    
    float sway = sin(time.r * speed + inPos.y * frequency + phase) * swayStrength * swayFactor;

    vec3 displacedPos = inPos;
    displacedPos.x += sway; // or use displacedPos.z += sway; or both

    vec4 worldPos = modelMatrix * vec4(displacedPos, 1.0);
    vec3 worldNormal = normalize(vec3(normalMatrix * vec4(inNormal, 0.0)));

    outUv = inUV;
    outFragPos = worldPos.xyz;
    outNormal = worldNormal;
    outDrawId = currentId;

    gl_Position = projection * view * worldPos;
}