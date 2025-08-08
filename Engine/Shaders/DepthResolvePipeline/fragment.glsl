#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 TexCoords;

uniform texture2DMS screenTexture;
uniform sampler screenTexture_sampler;

void main()
{
    ivec2 texSize = textureSize(screenTexture); 
    ivec2 texelCoord = ivec2(TexCoords * texSize);

    float minDepth = 1.0;

    // Resolve by picking the nearest depth
    for (int i = 0; i < SAMPLER_COUNT; ++i) {
        float depth = texelFetch(screenTexture, texelCoord, i).r;
        minDepth = min(minDepth, depth);
    }

    gl_FragDepth = minDepth;
}
