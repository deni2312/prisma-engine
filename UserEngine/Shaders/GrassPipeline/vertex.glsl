#version 460 core

layout(location = 0) in vec3 aPos;         // Vertex position
layout(location = 1) in vec3 normal;        // Vertex normal
layout(location = 2) in vec2 aTexCoords;    // Texture coordinates

out vec2 TexCoords;
out vec3 color;

struct GrassPosition {
    mat4 direction;
    mat4 position;
};

// Output: Culled grass positions and size
layout(std430, binding = 16) buffer GrassCull
{
    GrassPosition grassCull[];        // Positions of culled instances
};

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;
uniform float percent;
uniform float time;  // Time variable to control wind movement

void main()
{
    TexCoords = aTexCoords;

    mat4 currentModel = grassCull[gl_InstanceID].position * model;

    // Fetch the current model matrix for this instance of grass
    vec4 grassModel = currentModel * vec4(aPos, 1.0);


    // Wind effect: Apply a sinusoidal oscillation based on time and position
    float windStrength = 0.2;     // Control the strength of the wind effect
    float windSpeed = 2.0;        // Control the speed of the wind oscillation
    float windFrequency = 3.0;    // Frequency of the wind's sway

    // Height-based influence: Scale the wind influence by height (y-coordinate)
    float heightFactor = clamp(aPos.y, 0.0, 1.0);  // Values between 0 and 1 (base to tip)

    // Sway based on position, time, and heightFactor
    vec3 windOffset = vec3(
        sin(grassModel.y * windFrequency + time * windSpeed) * windStrength * heightFactor,
        0.0,
        cos(grassModel.y * windFrequency + time * windSpeed) * windStrength * heightFactor
    );

    // Adjust the grass position by the windOffset
    grassModel = grassModel + vec4(windOffset,0.0);

    // For lighting/shading effects (based on normals and currentPercent)
    color = vec3(0.01, 0.5*(aPos.y / percent ) / 100, 0.01);

    // Apply view, projection, and model matrices to get the final position
    gl_Position = projection * view * grassModel;
}
