#version 460 core
#extension GL_ARB_bindless_texture : enable
layout(quads, fractional_odd_spacing, ccw) in;

layout(bindless_sampler) uniform sampler2D heightMap;
uniform mat4 model;
uniform float mult;
uniform float shift;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

in vec2 TextureCoord[];

out float Height;

out vec3 FragPos;

out vec3 Normal;

out vec2 textureCoord;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = TextureCoord[0];
    vec2 t01 = TextureCoord[1];
    vec2 t10 = TextureCoord[2];
    vec2 t11 = TextureCoord[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    textureCoord = (t1 - t0) * v + t0;

    Height = texture(heightMap, textureCoord).y * mult + shift;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize(vec4(cross(vVec.xyz, uVec.xyz), 0));
    Normal = vec3(normal);
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0 + normal * Height;
    FragPos = vec3(model * vec4(p.xyz, 1.0));
    gl_Position = projection * view * model * p;
}