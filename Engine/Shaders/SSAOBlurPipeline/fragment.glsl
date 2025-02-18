#version 460 core

out float FragColor;

in vec2 UV;

layout(bindless_sampler) uniform sampler2D ssaoInput;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -4; x < 4; ++x) 
    {
        for (int y = -4; y < 4; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, UV + offset).r;
        }
    }
    FragColor = 1;
}  