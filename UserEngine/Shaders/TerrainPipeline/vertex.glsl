#version 460 core
layout(location = 0) in vec3 aPos;

out vec2 TexCoords;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

out vec3 FragPos;

out vec3 Normal;

uniform mat4 model;

uniform float textureScaling;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(mat3(model))));
    Normal = normalMatrix * vec3(1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}