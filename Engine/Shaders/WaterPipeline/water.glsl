layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Vertex {
    vec4 position;
    vec4 normal;
    vec4 texCoords;
    vec4 tangent;
    vec4 bitangent;
};

layout(binding=0) buffer WaterMesh {
    Vertex waterMesh_data[];
};

uniform Constants {
    vec4 time;
    float waveAmplitude;
    float waveFrequency;
    float waveSpeed;
    float padding;
};

// Optional: wave direction struct
struct Wave {
    float amplitude;
    float frequency;
    float speed;
    vec2 direction; // x and z components of wave direction
};

float computeWave(vec2 positionXZ, float time, Wave wave) {
    float phase = dot(positionXZ, wave.direction) * wave.frequency + time * wave.speed;
    return sin(phase) * wave.amplitude;
}
// GLSL 2D Perlin noise function
float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float grad(int hash, float x, float y) {
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

vec2 permute(vec2 x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

float perlinNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float u = fade(f.x);
    float v = fade(f.y);

    int a = int(mod(i.x + i.y * 57.0, 256.0));
    int b = int(mod(i.x + 1.0 + i.y * 57.0, 256.0));
    int c = int(mod(i.x + (i.y + 1.0) * 57.0, 256.0));
    int d = int(mod(i.x + 1.0 + (i.y + 1.0) * 57.0, 256.0));

    float res = lerp(
        lerp(grad(a, f.x, f.y), grad(b, f.x - 1.0, f.y), u),
        lerp(grad(c, f.x, f.y - 1.0), grad(d, f.x - 1.0, f.y - 1.0), u),
        v
    );
    return res;
}

float getTotalWaveDisplacement(vec2 positionXZ, float timeValue,
                               float amp, float freq, float speed) {
    Wave wave1 = Wave(amp * 0.6, freq, speed, normalize(vec2(1.0, 0.3)));
    Wave wave2 = Wave(amp * 0.4, freq * 1.5, speed * 1.2, normalize(vec2(0.5, -1.0)));
    Wave wave3 = Wave(amp * 0.3, freq * 2.0, speed * 0.8, normalize(vec2(-0.8, 0.6)));

    float totalWave = 0.0;
    totalWave += computeWave(positionXZ, timeValue, wave1);
    totalWave += computeWave(positionXZ, timeValue, wave2);
    totalWave += computeWave(positionXZ, timeValue, wave3);

    // Add Perlin noise for surface detail
    float noiseScale = 0.5;
    float noiseStrength = 0.2;
    float perlin = perlinNoise(positionXZ * noiseScale + vec2(timeValue * 0.1));
    totalWave += perlin * noiseStrength;

    return totalWave;
}


void main()
{
    uint tileIndex = gl_GlobalInvocationID.x;
    Vertex v = waterMesh_data[tileIndex];

    vec2 posXZ = v.position.xz;
    float timeValue = time.r; // Extract the float time value

    // Compute the wave displacement at the current vertex
    float totalWave = getTotalWaveDisplacement(posXZ, timeValue,
                                               waveAmplitude, waveFrequency, waveSpeed);
    v.position.y = totalWave;

    // --- Normal Computation using Finite Differences ---
    // Choose a small epsilon for finite differences
    float epsilon = 0.01;

    // Calculate displacement at slightly offset positions
    vec2 posXZ_plusX = posXZ + vec2(epsilon, 0.0);
    vec2 posXZ_minusX = posXZ - vec2(epsilon, 0.0);
    vec2 posXZ_plusZ = posXZ + vec2(0.0, epsilon);
    vec2 posXZ_minusZ = posXZ - vec2(0.0, epsilon);

    float H_plusX = getTotalWaveDisplacement(posXZ_plusX, timeValue,
                                             waveAmplitude, waveFrequency, waveSpeed);
    float H_minusX = getTotalWaveDisplacement(posXZ_minusX, timeValue,
                                              waveAmplitude, waveFrequency, waveSpeed);
    float H_plusZ = getTotalWaveDisplacement(posXZ_plusZ, timeValue,
                                             waveAmplitude, waveFrequency, waveSpeed);
    float H_minusZ = getTotalWaveDisplacement(posXZ_minusZ, timeValue,
                                              waveAmplitude, waveFrequency, waveSpeed);

    // Calculate the partial derivatives (slope)
    float dHdx = (H_plusX - H_minusX) / (2.0 * epsilon);
    float dHdz = (H_plusZ - H_minusZ) / (2.0 * epsilon);

    // The normal vector for a height field is (-dH/dx, 1, -dH/dz)
    // You'll need to normalize it to ensure it's a unit vector
    v.normal = normalize(vec4(-dHdx, 1.0, -dHdz, 0.0));
    // ----------------------------------------------------

    waterMesh_data[tileIndex] = v;
}