#version 460 core
layout (location = 0) in vec3 aPos;

layout(std430, binding = 1) buffer Matrices {
    mat4 modelMatrices[];
};


void main()
{
    gl_Position = modelMatrices[gl_DrawID] * vec4(aPos, 1.0);
}