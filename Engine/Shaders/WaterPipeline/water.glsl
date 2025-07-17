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

layout(std140, binding = 1) uniform Constants {
    vec4 time;
    float waveAmplitude;
    float waveFrequency;
    float waveSpeed;
    int size;
};

// Number of waves
const int NUM_WAVES = 4;

// Wave parameters stored separately (must be const if hardcoded)
const float waveAmplitudes[NUM_WAVES] = float[](0.3, 0.2, 0.1, 0.25);
const float waveFrequencies[NUM_WAVES] = float[](1.0, 0.8, 1.5, 1.2);
const float waveSpeeds[NUM_WAVES] = float[](1.0, 1.2, 0.8, 1.1);
const vec2 waveDirections[NUM_WAVES] = vec2[](
    vec2(1.0, 0.5),
    vec2(-0.5, 1.0),
    vec2(0.7, -1.0),
    vec2(-1.0, -0.3)
);

void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    uint tileIndex = y * uint(size) + x;

    Vertex v = waterMesh_data[tileIndex];
    vec3 pos = v.position.xyz;

    float height = 0.0;
    vec3 normal = vec3(0.0);

    for (int i = 0; i < NUM_WAVES; ++i) {
        vec2 dir = normalize(waveDirections[i]);
        float phase = dot(dir, pos.xz);
        float omega = waveFrequencies[i]*waveFrequency;
        float k = waveSpeeds[i]*waveSpeed;
        float amplitude = waveAmplitudes[i]*waveAmplitude;

        float wave = amplitude * sin(omega * phase + time.x * k);
        height += wave;

        // Calculate partial derivatives for normal approximation
        float dx = amplitude * omega * dir.x * cos(omega * phase + time.x * k);
        float dz = amplitude * omega * dir.y * cos(omega * phase + time.x * k);
        normal += vec3(-dx, 1.0, -dz);
    }

    pos.y = height;
    v.position.y = pos.y;

    v.normal = vec4(normalize(normal), 0.0);

    waterMesh_data[tileIndex] = v;
}