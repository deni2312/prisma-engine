#version 460 core
layout(location = 0) in vec3 aPos;

layout(std430, binding = 1) buffer Matrices {
    mat4 modelMatrices[];
};


layout(std430, binding = 29) buffer Ids {
    uint ids[];
};

void main()
{
    gl_Position = modelMatrices[ids[gl_DrawID]] * vec4(aPos, 1.0);
}