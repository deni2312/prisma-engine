#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 cameraPos;
uniform float iTime;
uniform vec2 resolution;
uniform vec3 lightDir;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

// I have no idea how to fix this dithering issue ill come back to it at some point.
// there also a few tricks, plus i know the dithering issue is being produced by the jitter
// and hash function

const int MAX_MARCHING_STEPS = 60;
const float MIN_DIST = 0.0;
const float MAX_DIST = 20.5;
const float PRECISION = 0.05;
const float MARCH_SIZE = 0.1;

//light
float marchSize = 0.075;

#define MAX_STEPS_LIGHTS 5
#define ABSORPTION_COEFFICIENT 1.
#define SCATTERING_ANISO 0.1
#define PI 3.14159265359


////////////////////////////////////////////////////


float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}



float HenyeyGreenstein(float g, float mu) {
    float gg = g * g;
    return (1.0 / (4.0 * PI)) * ((1.0 - gg) / pow(1.0 + gg - 2.0 * g * mu, 1.5));
}


float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = smoothstep(0., 1., f);

    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    float res = mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
        mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
        mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
            mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
    return res;
}

float fbm(vec3 p, bool lowRes) {
    vec3 q = p + iTime * 0.3 * vec3(0.6, -0.2, 0.3);


    float f = 0.8;
    float scale = 0.6;
    float factor = 2.05;
    int maxOctave = 5;

    if (lowRes) {
        maxOctave = 3;
    }

    for (int i = 0; i < maxOctave; i++) {
        f += scale * noise(q);
        q *= factor;
        factor += 0.1;
        scale *= 0.55;
    }

    return f;
}

////////////////////////////////////////////////////
float sdfBox(vec3 p, vec3 boxSize, vec3 translation, vec3 scale) {
    // Apply scaling to the input point
    p *= scale;

    // Apply translation to the input point
    p -= translation;

    // Compute the distance
    vec3 d = abs(p) - boxSize;

    // Calculate the signed distance
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}
float scene(vec3 p, bool lowRes) {

    // its p, radius, scale, pos
    // would be ideal i can control the times of scrolling
    // here ill do it later, cause 
    // i need to multiply the p value of fbm
    // by a vec3 which controls it
    p.xz = fract(p.xz) - 0.5;

    float distance = sdfBox(p,vec3(1.0),vec3(0,5,0),vec3(1.0));

    float f = fbm(p, lowRes);

    return -distance + f;
}


// Beer * Powder -> (exp(-dist * absorption  * 2.0f) but powder is just making it lighter
// and its supposed to be 1. - (exp(-dist * absorption  * 2.0f)... 
float BeersLaw(float dist, float absorption) {
    return exp((-dist * SCATTERING_ANISO)) * (exp(-dist * SCATTERING_ANISO * 2.0f));;
}

// Jitter function
float jitter(vec3 ro, vec3 rd, float time) {
    return hash(dot(ro + rd + vec3(time), vec3(12.9898, 78.233, 45.164)));
}

vec4 rayMarch(vec3 ro, vec3 rd, float start, float end, float jitterValue) {
    vec4 res = vec4(0.0);

    float totalTransmittance = 1.0;
    vec3 lightDirection = normalize(lightDir);
    float totalDensity = 0.0;

    float phase = HenyeyGreenstein(ABSORPTION_COEFFICIENT, dot(rd, lightDirection));

    bool hit = false; // Flag to track if we hit an object

    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        vec3 p = ro + res.a * rd;
        p += jitterValue * MARCH_SIZE * rd;

        float density = scene(p, false);

        if (density > PRECISION) {
            hit = true; // Set hit to true if we detect density

            for (int step = 0; step < MAX_STEPS_LIGHTS; step++) {
                p += lightDirection * marchSize;

                float lightSample = scene(p, true);
                totalDensity += lightSample;
            }
            float lightTransmittance = BeersLaw(totalDensity, SCATTERING_ANISO);

            float luminance = 0.12 + density * 1.0 / (4.0 * 3.14);
            totalTransmittance *= lightTransmittance;
            res.rgb += totalTransmittance * luminance;
        }

        res.a += MARCH_SIZE;

        if (res.a > MAX_DIST) break;
    }

    // Set alpha to 0.0 if no hit
    if (!hit) {
        discard;
    }

    return res;
}


void main()
{
    vec2 ndc = (gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0;

    vec3 color = vec3(0.0);


    // Camera ray origin
    // Calculate the aspect     // Calculate the aspect ratio
    float aspectRatio = resolution.x / resolution.y;

    // Generate the ray direction in normalized device coordinates
    vec4 clipSpacePos = vec4(ndc.x, ndc.y, -1.0, 1.0);  // -1 for z to look into the screen

    // Transform from clip space to camera/view space
    vec4 viewSpacePos = inverse(projection) * clipSpacePos;
    viewSpacePos.z = -1.0; // Ensure the direction points forward in camera space
    viewSpacePos.w = 0.0;  // A direction vector, not a position

    // Now transform from view space to world space
    vec3 rd = normalize((inverse(view) * viewSpacePos).xyz);

    // Ray origin in world space (camera position)
    vec3 ro = cameraPos;

    vec3 sunColor = vec3(1.0, 0.5, 0.3);
    vec3 sunDirection = normalize(lightDir);
    float sun = clamp(dot(sunDirection, rd), 0.0, 1.0);

    float jitterValue = jitter(ro, rd, iTime * 0.01);

    color = vec3(0.7, 0.7, 0.90) * 0.8;
    color -= 0.9 * vec3(0.90, 0.75, 0.90) * rd.y;
    color += sunColor * pow(sun, 1000.0);

    vec4 res = rayMarch(ro, rd, MIN_DIST, MAX_DIST, jitterValue);

    color = color + sunColor * res.rgb;

    FragColor = vec4(color, 1.0);
}
