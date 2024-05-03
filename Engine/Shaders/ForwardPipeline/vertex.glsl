#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec2 aBitangent;

out vec3 FragPos;

out vec2 TexCoords;

out vec3 Normal;
out vec4 shadowDirData[16];

flat out int drawId;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 1) buffer Matrices
{
    mat4 modelMatrices[];
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
    drawId = gl_DrawID;
    FragPos = vec3(modelMatrices[gl_DrawID] * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    mat3 normalMatrix = mat3(transpose(inverse(mat3(modelMatrices[gl_DrawID]))));
    Normal = normalMatrix * aNormal;
    for (int i=0; i < lenMat.r; i++) {
        shadowDirData[i] = shadowMatrices[i].shadow * vec4(FragPos, 1.0);
    }

    gl_Position = projection * view * vec4(FragPos, 1.0);
}