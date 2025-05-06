#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D hdrTexture;
uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

void main()
{
    const float gamma = 2.2;

    vec3 hdrText = texture(sampler2D(hdrTexture,screenTexture_sampler), TexCoords).rgb;
    vec3 screenText = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;

    hdrText += screenText;

    FragColor = vec4(hdrText, 1.0);
}
