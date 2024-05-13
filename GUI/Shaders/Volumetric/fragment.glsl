#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;

layout(std430, binding = 9) buffer CSMShadow
{
    float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    FragColor = vec4(color, 1.0);
}
