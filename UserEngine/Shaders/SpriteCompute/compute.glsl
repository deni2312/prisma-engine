layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Data{
    mat4 model;
    vec4 color;
};

buffer SpritesData
{
    Data modelSprite[]; 
};

uniform TimeData {
    float deltaTime;
    float time;
    int numParticles;
    float padding;
};

struct SpriteIdsData{
    int id;
    int width;
    int height;
    int speed;
};


buffer SpriteIds
{
    SpriteIdsData spriteId[];
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

// --------------------------------
// Hash / Random / Noise Utilities
// --------------------------------
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

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = random(uint(i.x + i.y * 57.0));
    float b = random(uint(i.x + 1.0 + i.y * 57.0));
    float c = random(uint(i.x + (i.y + 1.0) * 57.0));
    float d = random(uint(i.x + 1.0 + (i.y + 1.0) * 57.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

// --------------------------------
// Fancy Particle Effects
// --------------------------------

// Effect 1: Wavy Perlin Noise
vec3 effect_wavy_noise(vec2 base, float time)
{
    vec2 noisePos = base * 0.1 + time * 0.05;
    float offsetX = noise(noisePos * 1.5) * 5.0;
    float offsetY = noise(noisePos * 2.0 + vec2(0.0, time * 0.2)) * 3.0;
    float offsetZ = noise(noisePos * 1.8 + vec2(time * 0.3, 0.0)) * 5.0;
    return vec3(base.x + offsetX, offsetY, base.y + offsetZ);
}

// Effect 2: Vortex / Black Hole
vec3 effect_vortex(vec2 base, float time)
{
    float dist = length(base);
    float angle = atan(base.y, base.x) + time * (5.0 / (dist + 1.0));
    float radius = dist - time * 0.5;

    float x = radius * cos(angle);
    float z = radius * sin(angle);
    float y = sin(time * 2.0 + dist) * 2.0;

    return vec3(x, y, z);
}

// Effect 3: Heartbeat Explosion
vec3 effect_heartbeat(vec2 base, float time)
{
    float pulse = sin(time * 2.0) * 0.5 + 0.5;
    float spread = pulse * 20.0;

    float x = base.x * spread;
    float z = base.y * spread;
    float y = sin(length(base) * 5.0 - time * 10.0) * 2.0;

    return vec3(x, y, z);
}

// Effect 4: Wormhole Tunnel
vec3 effect_wormhole(vec2 base, float time)
{
    float spiralSpeed = time * 1.5;
    float angle = spiralSpeed + base.y * 0.1;
    float radius = 10.0 + sin(time + base.x) * 2.0;

    float x = radius * cos(angle);
    float z = radius * sin(angle);
    float y = base.y * 0.5 - time * 5.0;

    return vec3(x, y, z);
}

// Effect 5: MetaBlob Madness
vec3 effect_metablob(vec2 base, float time)
{
    float field = sin(base.x * 0.5 + time) + cos(base.y * 0.5 + time);
    float move = field * 5.0;

    float x = base.x + move;
    float z = base.y + move;
    float y = sin(time * 3.0 + field * 5.0) * 3.0;

    return vec3(x, y, z);
}

vec3 effect_tornado(vec2 base, float time)
{
    float dist = length(base) + 0.1;          // prevent division by zero
    float height = time * 5.0 + dist * 2.0;   // upward movement
    float swirlSpeed = 4.0 / dist;            // faster swirl near center
    float angle = atan(base.y, base.x) + time * swirlSpeed;

    // radius decreases with height for a funnel shape
    float radius = 1;

    // Add a bit of random horizontal sway
    float sway = sin(time * 2.0 + dist * 5.0) * 0.2;
    float x = (radius + sway) * cos(angle);
    float z = (radius + sway) * sin(angle);

    // Height cycles upward endlessly
    float y = mod(height, 30.0); // keeps looping

    return vec3(x, y, z);
}

void main()
{
    uint idx = gl_GlobalInvocationID.x;

    if (idx >= modelSprite.length()) return; // safety

    mat4 model = modelSprite[idx].model;

    float gridSize = ceil(sqrt(float(numParticles)));
    float halfGrid = gridSize * 0.5;

    float baseX = float(idx % uint(gridSize)) - halfGrid;
    float baseZ = float(idx / uint(gridSize)) - halfGrid;
    float baseY = 0.0;
    
    vec2 base = vec2(baseX, baseZ);

    // --- Pick the effect ---
    vec3 finalPos = effect_wavy_noise(base, time); // <<< just call different functions here

    mat4 translationMatrix = mat4(1.0);
    translationMatrix[3] = vec4(finalPos, 1.0);

    mat4 rotationMatrix = mat4(1.0); // Optional rotation later

    model = translationMatrix * rotationMatrix;

    modelSprite[idx].model = model;
    modelSprite[idx].color = vec4(8);
    omniData_data[idx].position = model[3];

    int idSprite = 0;
    spriteId[idx].id = idSprite;
    spriteId[idx].width = 5;
    spriteId[idx].height = 5;
    spriteId[idx].speed = 10;
}
