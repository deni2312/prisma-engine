#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
uniform vec2 texSize;

layout(rgba8,bindless_image) uniform image2D worleyTexture;

// Function to generate random points in a grid cell
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Main function for computing Worley noise
void main()
{
    // Calculate the coordinates of the current pixel
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    // Normalize coordinates in the range [0,1]
    vec2 uv = vec2(pixelCoords) / texSize;

    // Control the cell size of the grid
    int numCells = 10;
    vec2 gridPos = uv * numCells;
    ivec2 baseCell = ivec2(floor(gridPos)); // Base cell

    // Variables to track the minimum distance
    float minDist = 1.0;
    vec2 nearestFeaturePoint;

    // Search in the surrounding grid cells (3x3 neighborhood)
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            // Offset grid position
            ivec2 neighborCell = baseCell + ivec2(x, y);

            // Generate a random feature point within the neighboring cell
            vec2 featurePoint = vec2(neighborCell) + vec2(random(vec2(neighborCell.x, neighborCell.y)),
                random(vec2(neighborCell.y, neighborCell.x)));

            // Calculate the distance to the feature point
            float dist = distance(gridPos, featurePoint);

            // Keep track of the minimum distance
            if (dist < minDist) {
                minDist = dist;
                nearestFeaturePoint = featurePoint;
            }
        }
    }

    // Normalize the distance for better visualization (adjust based on cell count)
    minDist = clamp(minDist / float(numCells), 0.0, 1.0);

    // Output the Worley noise value as a grayscale color
    vec4 worleyColor = vec4(vec3(minDist), 1.0);

    // Write the result to the output texture
    imageStore(worleyTexture, pixelCoords, worleyColor);
}
