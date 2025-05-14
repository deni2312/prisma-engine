uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

layout(location = 0) in vec3 inPos;
//layout(location = 4) out mat3 outTBN;

uniform MeshData{
    mat4 model;
    vec4 color;
};

void main()
{
    gl_Position = projection * view * model * inPos;
}