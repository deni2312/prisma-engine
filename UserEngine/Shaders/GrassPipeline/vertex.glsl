#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

// Input: Grass positions (world space)
layout(std430, binding = 15) buffer GrassPositions
{
    vec4 grassPositions[];  // Positions of grass instances in world space
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
    newModel[3] = vec4(grassPositions[gl_InstanceID].x, grassPositions[gl_InstanceID].y, grassPositions[gl_InstanceID].z, 1.0);

    // Perform the transformation with the new model matrix
    gl_Position = projection * view * newModel * model * vec4(aPos.x, aPos.y, 0.0, 1.0);
}