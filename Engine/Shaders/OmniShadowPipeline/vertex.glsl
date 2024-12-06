#version 460 core
layout (location = 0) in vec3 aPos;

layout(std430, binding = 20) buffer MatricesCopy {
    mat4 modelMatricesCopy[];
};

void main()
{
    gl_Position = modelMatricesCopy[gl_DrawID] * vec4(aPos, 1.0);
}