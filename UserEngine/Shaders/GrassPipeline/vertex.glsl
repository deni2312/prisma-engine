#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 color;

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    mat4 grassCull[];        // Positions of culled instances
};


layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

uniform float percent;

void main()
{
    TexCoords = aTexCoords;

    mat4 grassModel = grassCull[gl_InstanceID] * model;

    mat3 normalMatrix = mat3(transpose(inverse(mat3(grassModel))));

    vec3 currentPercent = vec3(0, (aPos.y / percent) / 100, 0);

    color = normalMatrix * currentPercent;

    gl_Position = projection * view * grassModel * vec4(aPos, 1.0);
}