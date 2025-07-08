#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler; // Still used if you don't fully switch to samplerless

uniform Constants{
    vec3 resolution;
    float time;
};

// Placeholder for a noise function (you'd implement a real one like Perlin noise)
float noise(vec3 coord) {
    // ... complex noise calculation based on coord and time ...
    return fract(sin(dot(coord, vec3(12.9898, 78.233, 157.14))) * 43758.5453);
}

// Another placeholder for a more advanced noise, e.g., for detail
float fbm(vec3 coord) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < 4; ++i) { // Example: 4 octaves
        value += amplitude * noise(coord * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}


void main()
{
    // Start with the background from the screen texture
    vec3 baseColor = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;

    // Simulate 2D clouds for simplicity
    // Adjust these values to control cloud appearance
    vec2 cloudCoord = TexCoords * 5.0 + time * 0.05; // Scale and animate
    float cloudDensity = fbm(vec3(cloudCoord, time * 0.1)) * 1.5; // Use 3D noise for slight depth feel

    // Remap density to a more cloud-like range
    cloudDensity = smoothstep(0.3, 0.7, cloudDensity); // Sharpen the edges
    cloudDensity = pow(cloudDensity, 2.0); // Make denser parts more prominent

    // Basic cloud color (e.g., white with some sky influence)
    vec3 cloudColor = mix(baseColor, vec3(0.9, 0.9, 1.0), cloudDensity); // Blend with a light color

    // Simple lighting effect (very basic)
    // float lightInfluence = dot(normalize(vec3(1.0, 0.5, 0.0)), normalize(vec3(cloudDensity, cloudDensity, cloudDensity)));
    // cloudColor *= (0.7 + 0.3 * lightInfluence);

    FragColor = vec4(cloudColor, 1.0);
}