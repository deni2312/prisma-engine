#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

void main()
{
    const float gamma = 2.2;

    vec3 col = texture(screenTexture, TexCoords).rgb;

    col = vec3(1.0) - exp(-col * vec3(1.0));

    col = pow(col, vec3(1.0 / gamma));

    FragColor = vec4(col, 1.0);
}