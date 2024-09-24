#version 460 core

layout(location = 0) in vec3 aPos;         // Vertex position
layout(location = 1) in vec3 normal;        // Vertex normal
layout(location = 2) in vec2 aTexCoords;    // Texture coordinates

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
uniform float time;  // Time variable to control wind movement

void main()
{
    TexCoords = aTexCoords;

    // Fetch the current model matrix for this instance of grass
    mat4 grassModel = grassCull[gl_InstanceID] * model;

    // Calculate the normal matrix for shading
    mat3 normalMatrix = mat3(transpose(inverse(mat3(grassModel))));

    // Wind effect: Apply a sinusoidal oscillation based on time and position
    float windStrength = 0.2;     // Control the strength of the wind effect
    float windSpeed = 2.0;        // Control the speed of the wind oscillation
    float windFrequency = 3.0;    // Frequency of the wind's sway

    // Height-based influence: Scale the wind influence by height (y-coordinate)
    // The base (y = 0) should not move, while the tip (higher y values) should sway more.
    float heightFactor = clamp(aPos.y, 0.0, 1.0);  // Values between 0 and 1 (base to tip)

    // Sway based on position, time, and heightFactor
    vec3 windOffset = vec3(
        sin(aPos.y * windFrequency + time * windSpeed) * windStrength * heightFactor,
        0.0,
        cos(aPos.y * windFrequency + time * windSpeed) * windStrength * heightFactor
    );

    // Adjust the grass position by the windOffset
    vec3 swayedPos = aPos + windOffset;

    // Calculate percent-based color variation (for visual feedback)
    vec3 currentPercent = vec3(0, (swayedPos.y / percent) / 100, 0);
    color = normalMatrix * currentPercent;

    // Apply view, projection, and model matrices to get the final position
    gl_Position = projection * view * grassModel * vec4(swayedPos, 1.0);
}
