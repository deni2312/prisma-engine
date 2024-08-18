#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Buffer containing model matrices for each sprite
layout(std430, binding = 11) buffer SpritesData11
{
    mat4 modelSprite[];
};

// Uniform to control the gravity strength and delta time
uniform float deltaTime;  // Time elapsed between frames
uniform vec3 gravity = vec3(0.0, -9.8, 0.0);  // Gravity vector
uniform float spreadFactor = 5.0;  // Controls the amount of spread

// Random function to generate random numbers based on sprite index
float random(uint seed)
{
    return fract(sin(float(seed) * 43758.5453123) * 43758.5453123);
}

void main()
{
    // Get the current sprite index
    uint idx = gl_GlobalInvocationID.x;

    // Ensure we don't exceed the bounds of the buffer
    if (idx < modelSprite.length()) {
        // Extract the current translation (position) from the model matrix
        vec3 position = modelSprite[idx][3].xyz;

        // Generate a random direction for spreading
        vec3 randomDirection = vec3(
            random(idx + 1) - 0.5,  // X-axis random component
            random(idx + 2) - 0.5,  // Y-axis random component (can be small)
            random(idx + 3) - 0.5   // Z-axis random component
        );

        // Normalize the direction and scale it by spreadFactor and deltaTime
        randomDirection = normalize(randomDirection) * spreadFactor * deltaTime;

        // Update the position with gravity and random spread
        position += gravity * deltaTime + randomDirection;

        // Write the updated position back into the model matrix
        modelSprite[idx][3].xyz = position;

        if (modelSprite[idx][3].y < -10.0) {
            modelSprite[idx][3].x = 0.0;
            modelSprite[idx][3].y = 0.0;
            modelSprite[idx][3].z = 0.0;
        }

    }
}
