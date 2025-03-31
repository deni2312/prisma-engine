#version 460 core
layout(location = 0) in vec3 aPos;

flat out uint drawId;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 1) buffer Matrices {
    mat4 modelMatrices[];
};


void main()
{
    drawId = gl_DrawID;
    gl_Position = projection * view * modelMatrices[gl_DrawID]*vec4(aPos, 1.0);
}