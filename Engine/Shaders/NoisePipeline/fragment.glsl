#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

in vec2 TexCoords;

// Hash function
float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

// Noise function
float noise(in vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f * f * (3.0 - 2.0 * f);

    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    float res = mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
        mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
        mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
            mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
    return res;
}

void main() {
    // Create a 3D noise based on the 2D texture coordinates and a fixed z value
    float z = 0.0; // You can animate or vary this for more dynamic noise
    vec3 noiseInput = vec3(TexCoords, z);
    float col = noise(noiseInput);

    // Output the noise value as a grayscale color
    FragColor = vec4(vec3(col), 1.0);
}
