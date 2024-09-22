#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    vec4 grassCull[];        // Positions of culled instances
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

    // Constructing a new model matrix that applies translation based on grassCull
    mat4 newModel = mat4(1.0); // Identity matrix

    // Set translation part of the new model matrix using grassCull data
    newModel[3] = vec4(grassCull[gl_InstanceID].xyz, 1.0);

    // Perform the transformation with the new model matrix
    gl_Position = projection * view * newModel * model * vec4(aPos, 1.0);
}