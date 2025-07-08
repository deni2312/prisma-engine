#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;


uniform ViewProjection {
    mat4 uView;
    mat4 uProjection;
    vec4 viewPos; // camera position in world space
};

uniform Constants {
    vec3 resolution;
    float time;
    vec4 cloudPosition; // This uniform is available but not used in the provided SDF.
};

#define SDF_DIST 0.01
#define RAYMARCH_STEPS 50
#define MAX_DIST 50.0

struct Ray {
    vec3 dir;
    vec3 origin;
};

// SDF: sphere
// This function assumes 'point' is in world space.
float GetMinSceneDistanceFromPoint(vec3 point) {
    vec4 sphere = vec4(0.0, 1.0, 6.0, 1.0); // position.xyz, radius (in world space)
    return length(point - sphere.xyz) - sphere.w;
}

// Estimates the normal of the SDF at a given point in world space.
vec3 estimateNormal(vec3 p) {
    vec2 eps = vec2(0.01, 0.0);
    return normalize(vec3(
        GetMinSceneDistanceFromPoint(p + eps.xyy) - GetMinSceneDistanceFromPoint(p - eps.xyy),
        GetMinSceneDistanceFromPoint(p + eps.yxy) - GetMinSceneDistanceFromPoint(p - eps.yxy),
        GetMinSceneDistanceFromPoint(p + eps.yyx) - GetMinSceneDistanceFromPoint(p - eps.yyx)
    ));
}

// Calculates basic diffuse shading.
float calcShading(vec3 p) {
    vec3 lightPos = vec3(-5.0, 5.0, 2.0); // Light position in world space
    vec3 lightDir = normalize(lightPos - p);
    vec3 normal = estimateNormal(p);
    return clamp(dot(normal, lightDir), 0.0, 1.0);
}

struct RaymarchResult{
    float totalDistance;
    vec3 color;
    bool found;
};

// Performs raymarching to find the intersection with the SDF.
RaymarchResult raymarch(Ray r) {
    RaymarchResult result;
    result.totalDistance=0;
    result.found=false;
    for (int i = 0; i < RAYMARCH_STEPS; i++) {
        vec3 pos = r.origin + r.dir * result.totalDistance;
        float d = GetMinSceneDistanceFromPoint(pos); // 'pos' is in world space
        result.totalDistance += d;
        if (d < SDF_DIST){
            result.found=true;
            break;
        }
        
        if(result.totalDistance > MAX_DIST){
            result.found=false;
            break;
        }
    }
    return result;
}

void main() {

    vec2 fragPos=gl_FragCoord.xy;
    fragPos.y=resolution.y-fragPos.y;
    // Calculate normalized device coordinates (NDC) from fragment coordinates.
    vec2 uv = (fragPos / resolution.xy) * 2.0 - 1.0;

    // Reconstruct clip space position. For raycasting, z=1 (far plane) is common.
    vec4 clipPos = vec4(uv, 1.0, 1.0);

    // Transform clip space direction to view space.
    // The inverse of the projection matrix maps from clip space to view space.
    vec4 viewDirH = inverse(uProjection) * clipPos;
    // Perspective division to get a 3D vector in view space.
    vec3 rayDirView = normalize(viewDirH.xyz / viewDirH.w);

    // The ray origin in view space is (0,0,0) if the camera is at the origin of view space.
    // However, the `viewPos` uniform gives the camera's world position.
    // We need to construct the ray in world space to interact with the SDF defined in world space.

    // Calculate the inverse of the view matrix to transform from view space to world space.
    mat4 inverseView = inverse(uView);

    // Ray origin in world space: This is simply the camera's position.
    // The `viewPos` uniform is already the camera's position in world space.
    vec3 rayOriginWorld = viewPos.xyz;

    // Ray direction in world space: Transform the view space ray direction by the inverse view matrix
    // (treating it as a direction vector, so the translation part of the matrix is ignored).
    vec3 rayDirWorld = normalize((inverseView * vec4(rayDirView, 0.0)).xyz); // 0.0 for direction vector

    Ray ray;
    ray.origin = rayOriginWorld;
    ray.dir = rayDirWorld;

    RaymarchResult dist = raymarch(ray);

    if (dist.found) {
        // Calculate the hit point in world space.
        vec3 hitPoint = ray.origin + ray.dir * dist.totalDistance;
        float diffuse = calcShading(hitPoint);
        FragColor = vec4(vec3(diffuse), 1.0); // Render the SDF with shading
        vec4 clipSpaceHit = uProjection * uView * vec4(hitPoint, 1.0);
        gl_FragDepth = (clipSpaceHit.z / clipSpaceHit.w); // Perspective divide to get NDC Z
        return;
    } 
    discard;
   
}