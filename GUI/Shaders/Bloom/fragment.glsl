#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

void main()
{
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // Sobel edge detection
    vec3 sobel = vec3(0.0);
    sobel += texture(screenTexture, TexCoords + texelSize * vec2(-1, -1)).rgb * 1.0;
    sobel += texture(screenTexture, TexCoords + texelSize * vec2(-1, 0)).rgb * 2.0;
    sobel += texture(screenTexture, TexCoords + texelSize * vec2(-1, 1)).rgb * 1.0;
    sobel += texture(screenTexture, TexCoords + texelSize * vec2(1, -1)).rgb * -1.0;
    sobel += texture(screenTexture, TexCoords + texelSize * vec2(1, 0)).rgb * -2.0;
    sobel += texture(screenTexture, TexCoords + texelSize * vec2(1, 1)).rgb * -1.0;

    float sobelLength = length(sobel);
    vec3 edge = vec3(step(0.4, sobelLength)); // Adjust the threshold for edge detection

    // Reduce number of colors
    color = floor(color * 5.0) / 5.0;

    // Combine color and edge
    color = mix(color, vec3(0.0), edge);

    FragColor = vec4(color, 1.0);
}
