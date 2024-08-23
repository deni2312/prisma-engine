#version 460 core
layout(location = 0) in vec3 aPos;

out vec3 WorldPos;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

void main()
{
    WorldPos = aPos;
    gl_Position = projection * view * model * vec4(WorldPos, 1.0);
}