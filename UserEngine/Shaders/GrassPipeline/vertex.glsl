#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    mat4 grassCull[];        // Positions of culled instances
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

    gl_Position = projection * view * grassCull[gl_InstanceID] * model * vec4(aPos, 1.0);
}