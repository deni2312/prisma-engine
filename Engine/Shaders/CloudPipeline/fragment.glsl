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
    vec4 lightDirection;
    vec4 cloudColor;
    ivec4 type;
    float time;
    int maxSteps;
    float marchSize;
    float maxDistance;
};

#define MAX_STEPS 64
#define MARCH_SIZE 0.16
#define MAX_DISTANCE 50


struct Ray {
    vec3 origin;
    vec3 dir;
};

struct RaymarchResult {
    bool found;
    float totalDistance;
    vec4 color;
};

// SDF: sphere
// This function assumes 'point' is in world space.
float sdSphere(vec3 point,float size) {
    vec4 sphere = vec4(0.0, 0.0, 0.0,size); // position.xyz, radius (in world space)
    return length(point - sphere.xyz) - sphere.w;
}

float sdBox(vec3 p, vec3 size) {
    vec3 d = abs(p) - size;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

// Donut SDF - centered at `pos`, with major radius R and minor radius r
float sdDonut(vec3 p, float R) {
    vec2 t = vec2(length(p.xz) - R, p.y); // flatten to 2D donut cross-section
    return length(t) - 0.25;
}

float sdInterface(vec3 point,vec3 size){
    vec3 position=point-vec3(cloudPosition);
    if(type.r==0){
        return sdBox(position,size);
    }else if(type.r==1){
        return sdSphere(position,size.x);
    }else if(type.r==2){
        return sdDonut(position,size.x);
    }
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
    float currentDepth=0;
    vec3 size=vec3(1);
    RaymarchResult result;
    result.found=false;

    for (int i = 0; i < maxSteps; i++) {
        p = ray.origin + depth * ray.dir;
        float density = scene(p,size);

        if (density > 0.0) {
            if(!result.found){
                float f = fbm(p);
                currentDepth=depth+f;
                result.found=true;
            }
            
            float diffuse = clamp((scene(p,size) - scene(p + 0.3 * lightDirection.rgb,size)) / 0.3, 0.0, 1.0 );
            vec3 lin = vec3(0.60,0.60,0.75) * 1.1 + 0.8 * vec3(1.0,0.6,0.3) * diffuse;
            vec4 color = vec4(mix(vec3(1.0,1.0,1.0), vec3(0.0, 0.0, 0.0), density), density );
            color.rgb *= lin*cloudColor.rgb;
            color.rgb *= color.a;
            // In first phase is (1-0)*color, but color.a is density so next phase will be (1-density)*color decreasing the contribution
            accumColor += color * (1.0 - accumColor.a);
        }

        if(depth>maxDistance){
            break;
        }
        float stepSize=sdInterface(p,size);

        if(stepSize>0.01){
            depth += stepSize;
        }else{
            depth += marchSize;
        }
    }

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
    if(dist.found){
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
