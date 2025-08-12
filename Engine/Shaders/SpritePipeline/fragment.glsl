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
    int width;
    int height;
    int speed;
};

readonly buffer SpriteIds
{
    SpriteIdsData spriteId[];
};

void main()
{
    SpriteIdsData sid = spriteId[drawId];

    // Number of frames = width * height
    int totalFrames = sid.width * sid.height;

    // Compute current frame index
    // timeData is assumed to be in seconds, 10 fps for example:
    int frameIndex = int(floor(timeData*sid.speed)) % totalFrames;

    // Get row/column of frame
    int frameX = frameIndex % sid.width;
    int frameY = frameIndex / sid.width;

    // Size of one frame in texture UV space
    vec2 frameSize = vec2(1.0 / float(sid.width), 1.0 / float(sid.height));

    // Remap TexCoords to the correct frame
    vec2 frameUV = TexCoords * frameSize + vec2(frameX, frameY) * frameSize;

    // Sample from the correct sprite texture
    int texIndex = nonuniformEXT(sid.id);
    vec4 spriteTexture = texture(
        sampler2D(spriteTextures[texIndex], textureClamp_sampler),
        frameUV
    );

    // Alpha test
    if (spriteTexture.a < 0.1) {
        discard;
    }

    FragColor = spriteTexture * color;
}