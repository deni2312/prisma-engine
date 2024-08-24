#version 460 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;
in vec3 WorldPos;

uniform vec3 cameraPos;  // Position of the camera in world space
uniform vec3 lightDir;   // Direction of the light source (normalized)
uniform float time;      // Time to animate the cloud

// Noise function (can be replaced with any noise implementation)
float hash(vec3 p) {
    p = fract(p * 0.3183099 + vec3(1.0, 1.0, 1.0));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

// Simple 3D Perlin noise function
float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    return mix(mix(mix(hash(i + vec3(0, 0, 0)),
        hash(i + vec3(1, 0, 0)), f.x),
        mix(hash(i + vec3(0, 1, 0)),
            hash(i + vec3(1, 1, 0)), f.x), f.y),
        mix(mix(hash(i + vec3(0, 0, 1)),
            hash(i + vec3(1, 0, 1)), f.x),
            mix(hash(i + vec3(0, 1, 1)),
                hash(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}

float cloudDensity(vec3 p) {
    float density = 0.0;
    float scale = 1.0;
    for (int i = 0; i < 5; i++) {  // 5 octaves of noise
        density += noise(p * scale) / scale;
        scale *= 2.0;
        p *= 1.8; // Frequency multiplier
    }
    return density;
}

void main()
{
    vec3 rayDir = normalize(WorldPos - cameraPos);  // Calculate the ray direction
    vec3 rayOrigin = cameraPos;

    vec3 boxMin = vec3(-1.0);
    vec3 boxMax = vec3(1.0);

    // Ray-box intersection (AABB)
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    if (tNear > tFar || tFar < 0.0) {
        discard;  // No intersection with the box, discard the fragment
    }

    tNear = max(tNear, 0.0);  // Ensure tNear is positive

    float stepSize = 0.01;  // Step size for raymarching
    float totalDensity = 0.0;
    vec3 hitPoint;

    for (float t = tNear; t < tFar; t += stepSize) {
        hitPoint = rayOrigin + t * rayDir;

        // Move the hit point in noise space
        vec3 p = hitPoint * 2.0 + time * 0.1;

        // Get cloud density at this point
        float density = cloudDensity(p);

        // Accumulate density
        totalDensity += density * stepSize;
    }

    // Apply lighting to the cloud
    float lightIntensity = max(dot(normalize(lightDir), normalize(hitPoint - cameraPos)), 0.0);

    // Color the cloud based on accumulated density
    vec3 cloudColor = vec3(1.0) * totalDensity * lightIntensity;

    FragColor = vec4(cloudColor, totalDensity);
}