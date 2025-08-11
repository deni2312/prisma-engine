#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;
layout(location = 1) flat in int drawId;
layout(location = 2) flat in vec4 color;
layout(location = 3) flat in float timeData;

uniform sampler textureClamp_sampler;

uniform texture2D spriteTextures[];

struct SpriteIdsData{
    int id;
    int maxSize;
    int width;
    int height;
};

readonly buffer SpriteIds
{
    SpriteIdsData spriteId[];
};

void main()
{
    vec4 spriteTexture = texture(sampler2D(spriteTextures[nonuniformEXT(spriteId[drawId].id)],textureClamp_sampler), TexCoords);
    if (spriteTexture.a < 0.1) {
        discard;
    }
    FragColor = spriteTexture*color;
}