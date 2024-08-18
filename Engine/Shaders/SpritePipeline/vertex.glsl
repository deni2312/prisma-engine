#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

void main()
{
    // Extract the translation part of the view matrix (the camera position)
    mat4 viewNoRotation = mat4(1.0);
    viewNoRotation[3] = view[3]; // Copy the translation from the view matrix

    // Calculate the final position
    TexCoords = aTexCoords;
    gl_Position = projection * viewNoRotation * model * vec4(aPos, 1.0);
}
