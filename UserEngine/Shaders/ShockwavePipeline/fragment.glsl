#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

void main()
{
    // Sample the input texture
    vec4 texColor = texture(screenTexture, TexCoords);

    // Sepia tone conversion
    float r = texColor.r * 0.393 + texColor.g * 0.769 + texColor.b * 0.189;
    float g = texColor.r * 0.349 + texColor.g * 0.686 + texColor.b * 0.168;
    float b = texColor.r * 0.272 + texColor.g * 0.534 + texColor.b * 0.131;

    // Make sure values are clamped between 0 and 1
    r = clamp(r, 0.0, 1.0);
    g = clamp(g, 0.0, 1.0);
    b = clamp(b, 0.0, 1.0);

    // Output the final color
    FragColor = vec4(r, g, b, texColor.a);
}
