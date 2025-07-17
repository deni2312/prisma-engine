layout ( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;
struct Vertex {
   vec4 position;
   vec4 normal;
   vec4 texCoords;
   vec4 tangent;
   vec4 bitangent;
};

layout(binding=0)
buffer WaterMesh
{
    Vertex waterMesh_data[];
};

uniform Constants {
    vec4 time;
    float waveAmplitude;
    float waveFrequency;
    float waveSpeed;
    float padding;
};

void main()
{
    uint tileIndex = gl_GlobalInvocationID.x;
    Vertex v = waterMesh_data[tileIndex];

    // Animate the y-position with a sine wave based on time and x,z
    float wave = sin(waveFrequency * (v.position.x + v.position.z + waveSpeed * time.r));
    v.position.y = wave*waveAmplitude;

    // (Optional) Recalculate normals, tangents, etc., here if needed.

    // Write back the updated vertex
    waterMesh_data[tileIndex] = v;
}