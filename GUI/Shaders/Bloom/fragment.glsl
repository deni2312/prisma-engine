#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;


uniform Constants{
    ivec4 horizontal;
};

void main()
{
    float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
    vec2 tex_offset = 1.0 / textureSize(screenTexture, 0); // gets size of single texel
    
    vec3 currentTexture = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;

    vec3 result = currentTexture.rgb * weight[0];
    if (horizontal.r==1)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}
