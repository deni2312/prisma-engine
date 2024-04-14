#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

uniform float strength = 0.5;

void main()
{
    vec2 texCoordsNormalized = TexCoords * 2.0 - 1.0;
    float radius = length(texCoordsNormalized);
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // Vignette calculation
    float vignette = smoothstep(1.0, 1.0 - strength, radius);

    FragColor = vec4(color * vignette, 1.0);
}
