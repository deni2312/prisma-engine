#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;

in vec2 TexCoords;

// Function to create a random value based on a vec2 coordinate
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// Interpolation function
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Gradient function
float gradient(vec2 p, vec2 gradientVector) {
    return dot(gradientVector, p);
}

// Smoothstep function to smooth the noise
float smooth1(float t) {
    return t * t * (3.0 - 2.0 * t);
}

// Generate Perlin noise
float perlin(vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);

    vec2 topRight = vec2(1.0, 1.0);
    vec2 topLeft = vec2(0.0, 1.0);
    vec2 bottomRight = vec2(1.0, 0.0);
    vec2 bottomLeft = vec2(0.0, 0.0);

    float bottomLeftGrad = gradient(f - bottomLeft, vec2(rand(i), rand(i + vec2(1.0, 0.0))));
    float bottomRightGrad = gradient(f - bottomRight, vec2(rand(i + vec2(1.0, 0.0)), rand(i + vec2(0.0, 1.0))));
    float topLeftGrad = gradient(f - topLeft, vec2(rand(i + vec2(0.0, 1.0)), rand(i + vec2(1.0, 1.0))));
    float topRightGrad = gradient(f - topRight, vec2(rand(i + vec2(1.0, 1.0)), rand(i + vec2(0.0, 0.0))));

    float u = smooth1(f.x);
    float v = smooth1(f.y);

    float bottomInterpolation = lerp(bottomLeftGrad, bottomRightGrad, u);
    float topInterpolation = lerp(topLeftGrad, topRightGrad, u);

    return lerp(bottomInterpolation, topInterpolation, v);
}

void main() {
    // Scale coordinates for the noise function
    vec2 uv = TexCoords * 10.0;

    // Generate noise for each color channel
    float r = perlin(uv + vec2(0.0, 0.0));
    float g = perlin(uv + vec2(5.0, 5.0));
    float b = perlin(uv + vec2(10.0, 10.0));

    // Output the noise value as a color
    FragColor = vec4(r, g, b, 1.0);
}
