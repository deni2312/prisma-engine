#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D hdrTexture;
layout(bindless_sampler) uniform sampler2D screenTexture;

void main()
{
    const float gamma = 2.2;

    vec3 hdrText = texture(hdrTexture, TexCoords).rgb;
    vec3 screenText = texture(screenTexture, TexCoords).rgb;

    hdrText += screenText;

    hdrText = vec3(1.0) - exp(-hdrText * vec3(1.0));

    hdrText = pow(hdrText, vec3(1.0 / gamma));

    FragColor = vec4(hdrText, 1.0);
}
