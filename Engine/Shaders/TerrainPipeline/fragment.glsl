#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

in float Height;

in vec3 FragPos;
in vec3 Normal;

in vec2 textureCoord;

layout(bindless_sampler) uniform sampler2D grass;
layout(bindless_sampler) uniform sampler2D stone;
layout(bindless_sampler) uniform sampler2D snow;

uniform float mult;
uniform float shift;

void main()
{
    // Normalize height range (-16 to 48) to (0 to 1)
    float normalizedHeight = (Height + shift) / mult;

    // Define height thresholds for different layers
    float grassHeight = 0.3;  // Below 30% height, it's mostly grass
    float rockHeight = 0.7;   // Between 30% and 70%, it's rock
    float snowHeight = 1.0;   // Above 70%, snow

    // Blend factors between layers
    float grassFactor = smoothstep(0.0, grassHeight, normalizedHeight);
    float rockFactor = smoothstep(grassHeight, rockHeight, normalizedHeight);
    float snowFactor = smoothstep(rockHeight, snowHeight, normalizedHeight);

    // Sample textures based on TexCoords
    vec4 grassColor = texture(grass, textureCoord);
    vec4 rockColor = texture(stone, textureCoord);
    vec4 snowColor = texture(snow, textureCoord);

    // Blend textures based on height
    vec4 blendedColor = mix(grassColor, rockColor, rockFactor);  // Grass to Rock blend
    blendedColor = mix(blendedColor, snowColor, snowFactor);     // Rock to Snow blend

    // Additional effects based on normal direction (for snow accumulation on flat surfaces)
    float slopeFactor = dot(Normal, vec3(0.0, 1.0, 0.0));  // How flat the surface is
    slopeFactor = clamp(slopeFactor, 0.0, 1.0);            // Ensure slopeFactor is between 0 and 1

    // Accumulate more snow on flat surfaces
    blendedColor = mix(blendedColor, snowColor, slopeFactor * snowFactor);

    // Output final color
    FragColor = blendedColor;
}