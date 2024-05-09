#version 460 core

flat in int drawId;

out vec4 FragColor;

void main()
{
    FragColor = vec4(float(drawId)/256,0.0,0.0, 1.0);
}