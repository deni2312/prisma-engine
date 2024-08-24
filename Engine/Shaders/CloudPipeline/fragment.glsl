#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

void main()
{
    FragColor = vec4(color, 1.0);
}
