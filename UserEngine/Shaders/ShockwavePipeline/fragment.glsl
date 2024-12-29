#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

// Bindless texture for the screen
layout(bindless_sampler) uniform sampler2D screenTexture;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

// Uniforms for the shockwave effect
uniform vec3 shockCenter;        // The center of the shockwave in 3D space
uniform float time;              // Current time (controls the animation)
uniform float shockSpeed = 0.1;  // Speed at which the shockwave propagates
uniform float shockWidth = 0.1;  // Width of the shockwave ring
uniform float distortionStrength = 0.1; // Amount of distortion
uniform float maxDistance = 0.1; // Maximum distance for the distortion effect

void main()
{
    // Project the 3D shockwave center into screen space
    vec4 clipSpacePos = projection * view * vec4(shockCenter, 1.0);
    vec2 shockCenter2D = (clipSpacePos.xy / clipSpacePos.w) * 0.5 + 0.5; // NDC to screen coordinates

    // Calculate distance from the shock center
    float distance = length(TexCoords - shockCenter2D);

    // Apply effect only if within the maximum distance
    if (distance <= maxDistance)
    {
        // Compute the wave based on time
        float wave = sin((distance - time * shockSpeed) * 3.14159 / shockWidth);

        // Apply attenuation to the wave (only affecting a narrow ring)
        float attenuation = smoothstep(0.0, 0.5, 1.0 - abs(wave));

        // Calculate distortion offset
        vec2 distortion = normalize(TexCoords - shockCenter2D) * wave * distortionStrength * attenuation;

        // Apply distortion and clamp the coordinates to stay within [0, 1]
        vec2 distortedCoords = clamp(TexCoords + distortion, vec2(0.0), vec2(1.0));

        // Sample the screen texture with clamped distortion
        FragColor = texture(screenTexture, distortedCoords);
    }
    else
    {
        // If outside the maximum distance, sample the texture without distortion
        FragColor = texture(screenTexture, TexCoords);
    }
}
