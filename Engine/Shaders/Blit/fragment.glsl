#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

void main()
{
    FragColor = vec4(texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb, 1.0);
}
