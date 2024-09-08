#version 460 core

layout(std430, binding = 12) buffer SpritesData
{
    mat4 modelSprite[]; 
};

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

flat out int drawId;

uniform mat4 model;
uniform vec2 billboardSize;

void main()
{
    // Extract camera right and up vectors from the view matrix
    vec3 CameraRight_worldspace = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 CameraUp_worldspace = vec3(view[0][1], view[1][1], view[2][1]);

    // Calculate the center position of the particle or sprite in world space
    vec3 particleCenter_worldspace = vec3(model * modelSprite[gl_InstanceID][3]);

    drawId = gl_InstanceID;

    // Calculate the world space position of the vertex
    vec3 vertexPosition_worldspace = particleCenter_worldspace
        + CameraRight_worldspace * aPos.x * billboardSize.x
        + CameraUp_worldspace * aPos.y * billboardSize.y;

    // Set the texture coordinates for the fragment shader
    TexCoords = aTexCoords;

    // Project the vertex position to clip space
    gl_Position = projection * view * vec4(vertexPosition_worldspace, 1.0);
}
