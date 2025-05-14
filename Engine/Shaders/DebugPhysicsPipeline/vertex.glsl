uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

layout(location = 0) in vec3 inPos;
//layout(location = 4) out mat3 outTBN;

void main()
{
    gl_Position = projection * view * vec4(inPos,1);
}