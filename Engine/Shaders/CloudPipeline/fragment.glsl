#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

uniform ViewProjection {
    mat4 uView;
    mat4 uProjection;
    vec4 viewPos; // camera position
};

uniform Constants {
    vec3 resolution;
    float time;
    vec4 cloudPosition; // cloudPosition.rgb = world-space origin of cloud volume
};


#define SDF_DIST 0.01
#define RAYMARCH_STEPS 50
#define MAX_DIST 50.0

struct Ray {
    vec3 dir;
    vec3 origin;
};

// Signed Distance Function: sphere
float GetMinSceneDistanceFromPoint(vec3 point) {
    vec4 sphere = vec4(0.0, 1.0, 6.0, 1.0); // position.xyz, radius
    return length(point - sphere.xyz) - sphere.w;
}

// Estimate normal by sampling SDF gradient
vec3 estimateNormal(vec3 p) {
    vec2 eps = vec2(0.01, 0.0);
    return normalize(vec3(
        GetMinSceneDistanceFromPoint(p + eps.xyy) - GetMinSceneDistanceFromPoint(p - eps.xyy),
        GetMinSceneDistanceFromPoint(p + eps.yxy) - GetMinSceneDistanceFromPoint(p - eps.yxy),
        GetMinSceneDistanceFromPoint(p + eps.yyx) - GetMinSceneDistanceFromPoint(p - eps.yyx)
    ));
}

// Basic diffuse shading
float calcShading(vec3 p) {
    vec3 lightPos = vec3(-5.0, 5.0, 2.0);
    vec3 lightDir = normalize(lightPos - p);
    vec3 normal = estimateNormal(p);
    return clamp(dot(normal, lightDir), 0.0, 1.0);
}

// Raymarching loop
float raymarch(Ray r) {
    float totalDist = 0.0;
    for (int i = 0; i < RAYMARCH_STEPS; i++) {
        vec3 pos = r.origin + r.dir * totalDist;
        float d = GetMinSceneDistanceFromPoint(pos);
        totalDist += d;
        if (d < SDF_DIST || totalDist > MAX_DIST) break;
    }
    return totalDist;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * resolution.xy) / resolution.y;
    uv.y = -uv.y; // <- Vulkan Y-flip here

    vec4 color=texture(sampler2D(screenTexture,screenTexture_sampler),TexCoords);

    Ray ray;
    ray.origin = vec3(0.0, 1.0, 0.0);
    ray.dir = normalize(vec3(uv, 1.0));

    float dist = raymarch(ray);

    if (dist < MAX_DIST) {
        vec3 hitPoint = ray.origin + ray.dir * dist;
        float diffuse = calcShading(hitPoint);
        FragColor = vec4(vec3(diffuse), 1.0);
    } else {
        FragColor = vec4(color.xyz,viewPos.x); // background
    }
}
