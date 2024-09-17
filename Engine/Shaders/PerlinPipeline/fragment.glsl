#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

uniform vec2 resolution;

vec2 grad(ivec2 z)  // replace this anything that returns a random vector
{
    // 2D to 1D  (feel free to replace by some other)
    int n = z.x + z.y * 11111;

    // Hugo Elias hash (feel free to replace by another one)
    n = (n << 13) ^ n;
    n = (n * (n * n * 15731 + 789221) + 1376312589) >> 16;
    n &= 7;
    vec2 gr = vec2(n & 1, n >> 1) * 2.0 - 1.0;
    return (n >= 6) ? vec2(0.0, gr.x) :
        (n >= 4) ? vec2(gr.x, 0.0) :
        gr;
}

float noise(in vec2 p)
{
    ivec2 i = ivec2(floor(p));
    vec2 f = fract(p);

    vec2 u = f * f * (3.0 - 2.0 * f); // feel free to replace by a quintic smoothstep instead

    return mix(mix(dot(grad(i + ivec2(0, 0)), f - vec2(0.0, 0.0)),
        dot(grad(i + ivec2(1, 0)), f - vec2(1.0, 0.0)), u.x),
        mix(dot(grad(i + ivec2(0, 1)), f - vec2(0.0, 1.0)),
            dot(grad(i + ivec2(1, 1)), f - vec2(1.0, 1.0)), u.x), u.y);
}

void main() {
    // Scale the texture coordinates to control the frequency of the noise
    vec2 pos = gl_FragCoord.xy / resolution;  // Increase the scale for larger patterns

    // Get the noise value at the current position
    float noiseValue = 0;

    pos *= 8.0;
    mat2 m = mat2(1.6, 1.2, -1.2, 1.6);
    vec2 origPos = pos;

    noiseValue = 0.5000 * noise(pos); pos = m * pos;
    noiseValue += 0.2500 * noise(pos); pos = m * pos;
    noiseValue += 0.1250 * noise(pos); pos = m * pos;
    noiseValue += 0.0625 * noise(pos); pos = m * pos;

    noiseValue = 0.5 + 0.5 * noiseValue;

    // Calculate the gradient of the noise (approximate partial derivatives)
    float epsilon = 0.001; // Small offset for numerical derivative

    // Sample noise at nearby points
    float noiseX = noise(origPos + vec2(epsilon, 0.0));
    float noiseY = noise(origPos + vec2(0.0, epsilon));
    float noiseCenter = noise(origPos);

    // Compute partial derivatives (gradient)
    vec2 gradNoise = vec2(noiseX - noiseCenter, noiseY - noiseCenter) / epsilon;

    // Use the gradient to compute the normal (in the xy plane)
    vec3 normal = normalize(vec3(gradNoise, 1.0));

    // Output the normal map and the noise value as the fourth component
    FragColor = vec4(noiseValue,normal * 0.5 + 0.5); // Normalize the normal to [0, 1] range
}
