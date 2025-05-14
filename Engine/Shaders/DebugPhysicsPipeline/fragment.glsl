layout(location = 0) out vec4 FragColor;

uniform MeshData{
    vec4 color;
};

void main()
{
    FragColor = color;
}