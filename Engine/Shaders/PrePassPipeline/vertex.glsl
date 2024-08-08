#version 460 core
layout(location = 0) in vec3 aPos;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 1) readonly buffer Matrices
{
    mat4 modelMatrices[];
};

void main()
{
    vec3 FragPos = vec3(modelMatrices[gl_DrawID] * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
}