#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

layout(location = 0) in vec2 TexCoords;

uniform texture2D perlinTexture;
uniform sampler perlinTexture_sampler;

uniform ViewProjection {
    mat4 uView;
    mat4 uProjection;
    vec4 viewPos; // camera position in world space
};

uniform Constants {
    vec4 resolution;
    vec4 cloudPosition;
    float time;
    float amplitude;
    float frequency;
    float beers;
};

#define MAX_STEPS 128
#define MARCH_SIZE 0.08

// Access to perlin noise
#define uNoise perlinTexture
#define uTime time

struct Ray {
    vec3 origin;
    vec3 dir;
};

struct RaymarchResult {
    bool found;
    float totalDistance;
    vec4 color;
};

// SDF: Sphere
float sdSphere(vec3 p, float radius) {
    return length(p) - radius;
}

// Perlin noise
float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    vec2 uv = (p.xy + vec2(37.0, 239.0) * p.z) + f.xy;
    vec2 tex = textureLod(sampler2D(uNoise, perlinTexture_sampler), (uv + 0.5) / 256.0, 0.0).yx;

    return mix(tex.x, tex.y, f.z) * 2.0 - 1.0;
}

// Fractional Brownian Motion (FBM)
float fbm(vec3 p) {
    vec3 q = p + uTime * 0.5 * vec3(1.0, -0.2, -1.0);
    float f = 0.0;
    float scale = 0.5;
    float factor = 2.02;

    for (int i = 0; i < 6; i++) {
        f += scale * noise(q);
        q *= factor;
        factor += 0.21;
        scale *= 0.5;
    }

    return f;
}

// Scene function using sphere + noise
float scene(vec3 p) {
    float distance = sdSphere(p, 1.0);
    float f = fbm(p);
    return -distance + f;
}

// Raymarch with accumulation
RaymarchResult raymarch(Ray ray) {
    float depth = 0.0;
    vec3 p;
    vec4 accumColor = vec4(0.0);

    for (int i = 0; i < MAX_STEPS; i++) {
        p = ray.origin + depth * ray.dir;
        float density = scene(p);

        if (density > 0.0) {
            vec4 color = vec4(mix(vec3(1.0), vec3(0.0), density), density);
            color.rgb *= color.a;
            accumColor += color * (1.0 - accumColor.a);

            if (accumColor.a > 0.99) {
                break;
            }
        }

        depth += MARCH_SIZE;
    }

    RaymarchResult result;
    result.found = accumColor.a > 0.01;
    result.totalDistance = depth;
    result.color = accumColor;

    return result;
}

void main() {
    vec2 fragPos = gl_FragCoord.xy;
    fragPos.y = resolution.y - fragPos.y;
    vec2 uv = (fragPos / resolution.xy) * 2.0 - 1.0;

    vec4 clipPos = vec4(uv, 1.0, 1.0);
    vec4 viewDirH = inverse(uProjection) * clipPos;
    vec3 rayDirView = normalize(viewDirH.xyz / viewDirH.w);

    mat4 inverseView = inverse(uView);
    vec3 rayOriginWorld = viewPos.xyz;
    vec3 rayDirWorld = normalize((inverseView * vec4(rayDirView, 0.0)).xyz);

    Ray ray;
    ray.origin = rayOriginWorld;
    ray.dir = rayDirWorld;

    RaymarchResult dist = raymarch(ray);

    if (dist.found) {
        vec3 hitPoint = ray.origin + ray.dir * dist.totalDistance;
        vec4 clipSpaceHit = uProjection * uView * vec4(hitPoint, 1.0);
        gl_FragDepth = (clipSpaceHit.z / clipSpaceHit.w);

        float weight = clamp(pow(min(1.0, dist.color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragDepth * 0.9, 3.0), 1e-2, 3e3);

        accum = vec4(dist.color.rgb * dist.color.a, dist.color.a) * weight;
        reveal = dist.color.a;
        return;
    }

    discard;
}
