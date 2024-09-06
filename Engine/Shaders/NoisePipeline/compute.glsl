#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// Texture size (for storing the noise)
uniform vec3 texSize;

// 3D texture for storing the Worley noise
layout(rgba8, binding = 0) uniform image3D worleyTexture;

// Function to compute Worley noise (basic implementation)
float worleyNoise(vec3 pos) {
    // Generate noise with grid cells for Worley Noise
    const int numCells = 8; // Number of grid cells along each axis (can be modified)
    vec3 cellCoord = floor(pos * numCells);
    vec3 cellFrac = fract(pos * numCells);

    float minDist = 1.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                vec3 neighborCell = cellCoord + vec3(x, y, z);
                vec3 randomOffset = vec3(
                    fract(sin(dot(neighborCell, vec3(12.9898, 78.233, 45.164))) * 43758.5453),
                    fract(sin(dot(neighborCell, vec3(26.651, 45.716, 98.921))) * 23421.631),
                    fract(sin(dot(neighborCell, vec3(78.223, 56.234, 12.564))) * 78456.231)
                );

                vec3 neighborPos = (neighborCell + randomOffset) / numCells;
                float dist = length(neighborPos - pos);
                minDist = min(minDist, dist);
            }
        }
    }

    return minDist;
}

void main()
{
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);

    // Normalize the texture coordinates
    vec3 uv = vec3(storePos) / vec3(texSize);

    // Compute the Worley noise for the current position
    float noiseValue = worleyNoise(uv);

    // Store the Worley noise in the 3D texture
    vec4 outputColor = vec4(vec3(noiseValue), 1.0); // Alpha is set to 1.0
    imageStore(worleyTexture, storePos, outputColor);
}
