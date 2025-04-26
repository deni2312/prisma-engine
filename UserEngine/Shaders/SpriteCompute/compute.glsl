layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Buffer containing model matrices for each sprite
buffer SpritesData
{
    mat4 modelSprite[]; 
};

uniform TimeData {
    float deltaTime;
    float time;
    int numParticles;
    float padding;
};

buffer SpriteIds
{
    ivec4 spriteId[];
};

struct OmniData
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 far_plane;
    vec4 attenuation;
    int shadowIndex;
    float padding;
    float hasShadow;
    float radius;
};

buffer omniData {
    OmniData omniData_data[];
};

// Hash and random functions (keep yours)
uint hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}

float random(uint seed) {
    return float(hash(seed)) / 4294967295.0;
}

// Simple 2D Perlin-like noise based on hashing
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    // Four corners
    float a = random(uint(i.x + i.y * 57.0));
    float b = random(uint(i.x + 1.0 + i.y * 57.0));
    float c = random(uint(i.x + (i.y + 1.0) * 57.0));
    float d = random(uint(i.x + 1.0 + (i.y + 1.0) * 57.0));

    // Smooth interpolation
    vec2 u = f * f * (3.0 - 2.0 * f);

    // Mix the results
    return mix(a, b, u.x) +
           (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}


void main()
{
    uint idx = gl_GlobalInvocationID.x;

    mat4 model = modelSprite[idx];

    // --- Parametrizable layout ---
    float gridSize = ceil(sqrt(float(numParticles))); // how many particles per row/col
    float halfGrid = gridSize * 0.5;                    // to center around (0,0)

    // Base grid position
    float baseX = (float(idx % uint(gridSize)) - halfGrid);
    float baseZ = (float(idx / uint(gridSize)) - halfGrid);
    float baseY = 0.0;

    vec2 noisePos = vec2(baseX * 0.1, baseZ * 0.1) + time * 0.05;

    // Calculate noise-influenced displacement
    float offsetX = noise(noisePos * 1.5) * 5.0;
    float offsetY = noise(noisePos * 2.0 + vec2(0.0, time * 0.2)) * 3.0;
    float offsetZ = noise(noisePos * 1.8 + vec2(time * 0.3, 0.0)) * 5.0;

    vec3 finalPos = vec3(baseX + offsetX, baseY + offsetY, baseZ + offsetZ);

    // Create the translation matrix
    mat4 translationMatrix = mat4(1.0);
    translationMatrix[3] = vec4(finalPos, 1.0);

    // Optionally, you could add small random rotations here if you want more dynamic particles
    mat4 rotationMatrix = mat4(1.0); // No rotation for now

    model = translationMatrix * rotationMatrix;

    modelSprite[idx] = model;
    omniData_data[idx].position = model[3];

    int idSprite = 0;
    if (idx > 500) {
        idSprite = 1;
    }
    spriteId[idx].r = idSprite;
}
