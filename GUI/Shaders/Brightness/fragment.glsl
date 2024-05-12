#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

void main()
{
    vec3 result = texture(screenTexture, TexCoords).rgb;
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if (brightness < 1.0) {
        result = vec3(0.0, 0.0, 0.0);
    }
    FragColor = vec4(result, 1.0);
}
