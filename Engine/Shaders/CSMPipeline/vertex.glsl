#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec3 inPos;

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
    gl_Position = modelsData[gl_DrawIDARB].model * vec4(inPos, 1.0);
}