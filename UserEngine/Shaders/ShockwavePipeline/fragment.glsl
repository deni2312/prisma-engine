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
uniform vec3 shockCenter; // The center of the shockwave in 3D space
uniform float time;         // Current time (controls the animation)
uniform float shockSpeed=1;   // Speed at which the shockwave propagates
uniform float shockWidth=1;   // Width of the shockwave ring
uniform float distortionStrength=1; // Amount of distortion

void main()
{
    // Project the 3D shockwave center into screen space
    vec4 clipSpacePos = projection * view * vec4(shockCenter, 1.0);
    vec2 shockCenter = (clipSpacePos.xy / clipSpacePos.w) * 0.5 + 0.5; // NDC to screen coordinates

    // Calculate distance from the shock center
    float distance = length(TexCoords - shockCenter);

    // Compute the wave based on time
    float wave = sin((distance - time * shockSpeed) * 3.14159 / shockWidth);

    // Apply attenuation to the wave (only affecting a narrow ring)
    float attenuation = smoothstep(0.0, 0.5, 1.0 - abs(wave));

    // Calculate distortion offset
    vec2 distortion = normalize(TexCoords - shockCenter) * wave * distortionStrength * attenuation;

    // Sample the screen texture with distortion
    vec2 distortedCoords = TexCoords + distortion;
    vec4 color = texture(screenTexture, distortedCoords);

    // Output the final color
    FragColor = color;
}
