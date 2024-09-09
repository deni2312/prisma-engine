#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

in vec3 FragPos;

void main()
{
    FragColor = vec4(1.0);
}