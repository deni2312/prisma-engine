#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform texture2D volumetricTexture;
uniform sampler screenTexture_sampler;

void main()
{
    vec3 screen = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;
    vec3 volumetricTexture = texture(sampler2D(volumetricTexture,screenTexture_sampler), TexCoords).rgb;

    vec3 result = screen + volumetricTexture;

    FragColor = vec4(volumetricTexture,result.r);
}
