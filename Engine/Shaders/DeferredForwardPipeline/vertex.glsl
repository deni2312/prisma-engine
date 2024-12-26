#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec3 FragPos;

out vec2 TexCoords;

out vec3 Normal;

flat out uint drawId;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};


layout(std430, binding = 1) buffer Matrices {
    mat4 modelMatrices[];
};


layout(std430, binding = 29) buffer Ids {
    uint ids[];
};

void main()
{
    drawId = ids[gl_DrawID];
    FragPos = vec3(modelMatrices[drawId] * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    mat3 normalMatrix = mat3(transpose(inverse(mat3(modelMatrices[drawId]))));
    Normal = normalMatrix * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}