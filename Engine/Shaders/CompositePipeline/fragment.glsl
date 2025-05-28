#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

void main()
{
    vec3 result = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;
    //float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    FragColor = vec4(result, 1.0);
}
