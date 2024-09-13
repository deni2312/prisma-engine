#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D grassSprite;

void main()
{
    vec4 col = texture(grassSprite, TexCoords).rgba;
    if (col.a < 0.1) {
        discard;
    }

    FragColor = vec4(col.rgb, 1.0);
}