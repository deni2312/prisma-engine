#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D sprite;

void main()
{
    vec4 spriteTexture = texture(sprite, TexCoords);
    if (spriteTexture.a < 0.1) {
        discard;
    }
    FragColor = spriteTexture;
}