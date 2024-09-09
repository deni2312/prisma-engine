#version 460 core
layout(location = 0) in vec3 aPos;

out vec2 TexCoords;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

out vec3 FragPos;

uniform mat4 model;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
}