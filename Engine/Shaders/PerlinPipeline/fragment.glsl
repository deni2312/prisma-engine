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
    vec2 pos = gl_FragCoord.xy/resolution;  // Increase the scale for larger patterns

    // Get the noise value
    float noiseValue = 0;

    pos *= 8.0;
    mat2 m = mat2(1.6, 1.2, -1.2, 1.6);
    noiseValue = 0.5000 * noise(pos); pos = m * pos;
    noiseValue += 0.2500 * noise(pos); pos = m * pos;
    noiseValue += 0.1250 * noise(pos); pos = m * pos;
    noiseValue += 0.0625 * noise(pos); pos = m * pos;

    noiseValue = 0.5 + 0.5 * noiseValue;

    // Map noise value to grayscale
    vec3 col = vec3(noiseValue);  // Normalize noise output from [-1,1] to [0,1]

    // Output the color with full opacity
    FragColor = vec4(col, 1.0);
}