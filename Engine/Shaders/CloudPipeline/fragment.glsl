#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

layout(location = 0) in vec2 TexCoords;

//uniform texture2D perlinTexture;
//uniform sampler perlinTexture_sampler;

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

#define MAX_STEPS 64
#define MARCH_SIZE 0.1
#define MARCH_LONG 1
#define MAX_DISTANCE 50

const vec3 SUN_POSITION = vec3(1.0, 0.0, 0.0);


struct Ray {
    vec3 origin;
    vec3 dir;
};

struct RaymarchResult {
    float totalDistance;
    vec4 color;
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

#define BOX

float sdInterface(vec3 point,vec3 size){
#ifdef SPHERE
    return sdSphere(point,size.x);
#endif

#ifdef BOX
    return sdBox(point,size);
#endif

}

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

// Fractional Brownian Motion (FBM)
float fbm(vec3 p) {
    vec3 q = p + time * 0.5 * vec3(1.0, -0.2, -1.0);
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
float scene(vec3 p,vec3 size) {
    float distance = sdInterface(p,size);
    float f = fbm(p);
    return -distance + f;
}

// Raymarch with accumulation
RaymarchResult raymarch(Ray ray) {
    float depth = 0.0;
    vec3 p;
    vec4 accumColor = vec4(0.0);
    bool found=false;
    bool foundLong=false;
    float currentDepth=0;
    vec3 size=vec3(1);
    vec3 sunDirection = normalize(SUN_POSITION);

    for (int i = 0; i < MAX_STEPS; i++) {
        p = ray.origin + depth * ray.dir;
        float density = scene(p,size);

        if (density > 0.0) {
            if(!found){
                currentDepth=depth;
            }

            found=true;
            
            float diffuse = clamp((scene(p,size) - scene(p + 0.3 * sunDirection,size)) / 0.3, 0.0, 1.0 );
            vec3 lin = vec3(0.60,0.60,0.75) * 1.1 + 0.8 * vec3(1.0,0.6,0.3) * diffuse;
            vec4 color = vec4(mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), density), density );
            color.rgb *= lin;
            color.rgb *= color.a;
            accumColor += color * (1.0 - accumColor.a);

            if (accumColor.a > 0.95) {
                break;
            }
        }

        if(depth>MAX_DISTANCE){
            break;
        }

        depth += sdInterface(p,size);
    }

    RaymarchResult result;
    result.totalDistance = currentDepth;
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

    vec3 hitPoint = ray.origin + ray.dir * dist.totalDistance;
    vec4 clipSpaceHit = uProjection * uView * vec4(hitPoint, 1.0);
    gl_FragDepth = (clipSpaceHit.z / clipSpaceHit.w);

    float weight = clamp(pow(min(1.0, dist.color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragDepth * 0.9, 3.0), 1e-2, 3e3);

    accum = vec4(dist.color.rgb * dist.color.a, dist.color.a) * weight;
    reveal = dist.color.a;

}
