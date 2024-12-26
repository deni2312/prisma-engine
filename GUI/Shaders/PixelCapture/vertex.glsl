#version 460 core
layout(location = 0) in vec3 aPos;

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


layout(std430, binding = 29) buffer Ids {
    uint ids[];
};

void main()
{
    drawId = int(ids[gl_DrawID]);
    gl_Position = projection * view * modelMatrices[ids[gl_DrawID]]*vec4(aPos, 1.0);
}