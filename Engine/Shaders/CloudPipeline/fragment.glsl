#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform Constants{
    vec4 resolution;
};

void main()
{
    FragColor = resolution;
}
