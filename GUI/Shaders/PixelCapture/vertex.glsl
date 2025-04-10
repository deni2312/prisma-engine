#extension GL_ARB_shader_draw_parameters : enable

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

layout(location = 0) in vec3 inPos;

layout(location = 0) flat out int outDrawId;

struct MeshData
{
    mat4 model;
    mat4 normal;
};


readonly buffer models{
    MeshData modelsData[];
};

void main()
{
    gl_Position = projection*view*modelsData[gl_DrawIDARB].model * vec4(inPos, 1.0);
    outDrawId = gl_DrawIDARB;
}