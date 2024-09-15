#version 460 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;


layout(std430, binding = 15) buffer GrassPositions
{
    mat4 grassPositions[];
};

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * grassPositions[gl_InstanceID]* model * vec4(aPos.x, aPos.y, 0.0, 1.0);
}