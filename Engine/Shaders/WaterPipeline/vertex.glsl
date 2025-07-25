#extension GL_ARB_shader_draw_parameters : enable


uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inUV;
layout(location = 3) in vec4 inTangent;
layout(location = 4) in vec4 inBitangent;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outLocalPos;



struct ModelData{
    mat4 model;
    mat4 normal;
};

uniform ModelConstant{
    ModelData model;
};

void main()
{

    vec4 worldPos = model.model * vec4(inPos.xyz, 1.0);
    vec3 worldNormal = normalize(vec3(model.normal * vec4(inNormal.xyz, 0.0)));

    outUv = inUV.xy;
    outFragPos = worldPos.xyz;
    outNormal = worldNormal.xyz;
    outLocalPos=inPos.xyz;

    gl_Position = projection * view * worldPos;
}