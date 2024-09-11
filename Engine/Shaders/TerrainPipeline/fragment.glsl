#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

in float Height;

in vec3 FragPos;
in vec3 Normal;

void main()
{
    float h = (Height + 16) / 64.0f;
    FragColor = vec4(h, h, h, 1.0);
}