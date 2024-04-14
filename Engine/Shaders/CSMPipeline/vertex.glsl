#version 460 core
layout(location = 0) in vec3 aPos;

layout(std430, binding = 1) buffer Matrices
{
    mat4 modelMatrices[];
};

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * modelMatrices[gl_DrawID] * vec4(aPos, 1.0);
}