#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Buffer containing model matrices for each sprite
layout(std430, binding = 12) buffer SpritesData
{
    mat4 modelSprite[];
};

// Uniforms to control the tornado effect and delta time
uniform float deltaTime;       // Time elapsed between frames
uniform float time;            // Total time elapsed


layout(std430, binding = 14) buffer SpriteIds
{
    ivec4 spriteId[];
};

// A simple hash function to generate pseudo-random values
uint hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}

// Convert a hashed integer to a float in the range [0.0, 1.0]
float random(uint seed) {
    return float(hash(seed)) / 4294967295.0; // 2^32 - 1
}

void main()
{
    // Get the current sprite index
    uint idx = gl_GlobalInvocationID.x;

    // Ensure we don't exceed the bounds of the buffer
    if (idx < modelSprite.length()) {
        // Get the original model matrix for this sprite
        mat4 model = modelSprite[idx];

        // Calculate the position of the sprite in a circular pattern
        float rotationSpeed = 1.0;        // Speed of rotation around the Y-axis
        float angle = mod(time * rotationSpeed + idx * 0.1, 6.28318530718); // Angle, wrapped at 360 degrees (2 * PI)
        float radius = 0.5;   // Radius from the center, increases with sprite index
        float height = mod(time * 0.5 + idx * 0.05, 10.0); // Height over time, loops every 10 units

        // Calculate the new position in the XZ plane
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Set the new translation
        vec3 position = vec3(x, height, z);

        // Create rotation matrix around the Y-axis (up-axis)
        mat4 rotationMatrix = mat4(1.0);
        rotationMatrix[0][0] = cos(angle);
        rotationMatrix[0][2] = -sin(angle);
        rotationMatrix[2][0] = sin(angle);
        rotationMatrix[2][2] = cos(angle);

        // Create the translation matrix
        mat4 translationMatrix = mat4(1.0);
        translationMatrix[3] = vec4(position, 1.0);

        // Combine the translation and rotation
        model = translationMatrix * rotationMatrix;

        // Update the model matrix for this sprite
        modelSprite[idx] = model;
        int idSprite = 0;
        if (idx > 500) {
            idSprite = 1;
        }
        spriteId[idx].r = idSprite;
    }
}
