#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

in vec3 TexCoords;

layout(bindless_sampler) uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}