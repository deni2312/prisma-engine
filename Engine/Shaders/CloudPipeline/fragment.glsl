#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

layout(location = 0) in vec2 TexCoords;


uniform ViewProjection {
    mat4 uView;
    mat4 uProjection;
    vec4 viewPos; // camera position in world space
};

uniform Constants {
    vec4 resolution;
    vec4 cloudPosition; // This uniform is available but not used in the provided SDF.
    float time;
    float amplitude;
    float frequency;
    float beers;
};

#define SDF_DIST 0.01
#define RAYMARCH_STEPS 50
#define MAX_DIST 50.0
#define STEP_SIZE 0.01

struct Ray {
    vec3 dir;
    vec3 origin;
};

// SDF: sphere
// This function assumes 'point' is in world space.
float sdSphere(vec3 point,float size) {
    point=point-vec3(cloudPosition);
    vec4 sphere = vec4(0.0, 0.0, 0.0,size); // position.xyz, radius (in world space)
    return length(point - sphere.xyz) - sphere.w;
}

float sdBox(vec3 p, vec3 size) {
    p=p-vec3(cloudPosition);
    vec3 d = abs(p) - size;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

#define SPHERE

float sdInterface(vec3 point,vec3 size){
#ifdef SPHERE
    return sdSphere(point,size.x);
#endif

#ifdef BOX
    return sdBox(point,size);
#endif

}

struct RaymarchResult{
    float totalDistance;
    vec4 color;
    bool found;
};

// Simple 3D Perlin noise stub (replace with your preferred noise function)
float hash(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f*f*(3.0 - 2.0*f); // smootherstep

    float n = mix(
        mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), f.x),
            mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), f.x),
            mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), f.x), f.y), f.z);
    return n;
}

// Performs raymarching to find the intersection with the SDF.
RaymarchResult raymarch(Ray r) {
    RaymarchResult result;
    result.totalDistance=0;
    result.color=vec4(0);
    result.found=false;
    vec3 halfSize = vec3(1.0, 1.0,1.0);    // Square of size 2x2
    for (int i = 0; i < RAYMARCH_STEPS; i++) {
        vec3 pos = r.origin + r.dir * result.totalDistance;
        float d = sdInterface(pos,halfSize); // 'pos' is in world space
        result.totalDistance += d;
        if (d <= 0.01){
                result.found=true;
                while(d>=0){
                    pos = r.origin + r.dir * result.totalDistance;
                    d =  sdInterface(pos,halfSize); // 'pos' is in world space
                    result.totalDistance += STEP_SIZE;
                    if(result.totalDistance > MAX_DIST){
                        result.found=false;
                        break;
                    }
                }
                if(!result.found){
                    break;
                }

                float base=result.totalDistance;
                float radiusSize=1.0/STEP_SIZE;
                float density =0.0;
                //Raymarching
                while(d < 0){
                    pos = r.origin + r.dir * result.totalDistance;
                    //result.color+=vec3(1.0)*exp(-1*(abs(result.totalDistance-base)));
                    d =  sdInterface(pos,halfSize); // 'pos' is in world space
                    density += noise((pos-cloudPosition.xyz) * frequency * time) * amplitude; // Scale and amplitude of noise

                    result.totalDistance += STEP_SIZE;
                }
                result.color=result.color+vec4(1.0)-vec4(exp(-density*beers*(abs(result.totalDistance-base))));

                result.totalDistance=base;
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
        vec4 clipSpaceHit = uProjection * uView * vec4(hitPoint, 1.0);
        gl_FragDepth = (clipSpaceHit.z / clipSpaceHit.w); // Perspective divide to get NDC Z

        float weight = clamp(pow(min(1.0, dist.color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragDepth * 0.9, 3.0), 1e-2, 3e3);

        accum = vec4(dist.color.rgb * dist.color.a, dist.color.a) * weight;

        reveal=dist.color.a;


        return;
    } 
    discard;
   
}