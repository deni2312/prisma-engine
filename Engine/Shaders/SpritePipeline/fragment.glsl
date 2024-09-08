#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

in vec2 TexCoords;

struct SpriteTexture {
    sampler2D spriteTextures;
    vec2 padding;
};

layout(std430, binding = 13) buffer SpriteTextures
{
    SpriteTexture spriteTextures[];
};

layout(std430, binding = 14) buffer SpriteIds
{
    ivec4 spriteId[];
};

flat in int drawId;

void main()
{
    vec4 spriteTexture = texture(spriteTextures[spriteId[drawId].r].spriteTextures, TexCoords);
    if (spriteTexture.a < 0.1) {
        discard;
    }
    FragColor = spriteTexture;
}