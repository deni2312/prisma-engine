#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 cameraPos;
uniform float iTime;
uniform vec2 resolution;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

mat3 m = mat3(0.00, 0.80, 0.60,
    -0.80, 0.36, -0.48,
    -0.60, -0.48, 0.64);

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f * f * (3.0 - 2.0 * f);

    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    float res = mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
        mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
        mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
            mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
    return res;
}

float fbm(vec3 p)
{
    float f;
    f = 0.5000 * noise(p); p = m * p * 2.02;
    f += 0.2500 * noise(p); p = m * p * 2.03;
    f += 0.12500 * noise(p); p = m * p * 2.01;
    f += 0.06250 * noise(p);
    return f;
}
/////////////////////////////////////

float stepUp(float t, float len, float smo)
{
    float tt = mod(t += smo, len);
    float stp = floor(t / len) - 1.0;
    return smoothstep(0.0, smo, tt) + stp;
}

// iq's smin
float smin(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
}

float sdTorus(vec3 p, vec2 t)
{
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}


float map(in vec3 p)
{
    vec3 q = p - vec3(0.0, 0.5, 1.0) * iTime;
    float f = fbm(q);
    float s1 = 1.0 - length(p * vec3(0.5, 1.0, 0.5)) + f * 2.2;
    float s2 = 1.0 - length(p * vec3(0.1, 1.0, 0.2)) + f * 2.5;
    float torus = 1. - sdTorus(p * 2.0, vec2(6.0, 0.005)) + f * 3.5;
    float s3 = 1.0 - smin(smin(
        length(p * 1.0 - vec3(cos(iTime * 3.0) * 6.0, sin(iTime * 2.0) * 5.0, 0.0)),
        length(p * 2.0 - vec3(0.0, sin(iTime) * 4.0, cos(iTime * 2.0) * 3.0)), 4.0),
        length(p * 3.0 - vec3(cos(iTime * 2.0) * 3.0, 0.0, sin(iTime * 3.3) * 7.0)), 4.0) + f * 2.5;

    float t = mod(stepUp(iTime, 4.0, 1.0), 4.0);

    float d = mix(s1, s2, clamp(t, 0.0, 1.0));
    d = mix(d, torus, clamp(t - 1.0, 0.0, 1.0));
    d = mix(d, s3, clamp(t - 2.0, 0.0, 1.0));
    d = mix(d, s1, clamp(t - 3.0, 0.0, 1.0));

    return min(max(0.0, d), 1.0);
}

float jitter;

#define MAX_STEPS 48
#define SHADOW_STEPS 8
#define VOLUME_LENGTH 15.
#define SHADOW_LENGTH 2.

vec4 cloudMarch(vec3 p, vec3 ray)
{
    float density = 0.;

    float stepLength = VOLUME_LENGTH / float(MAX_STEPS);
    float shadowStepLength = SHADOW_LENGTH / float(SHADOW_STEPS);
    vec3 light = normalize(vec3(1.0, 2.0, 1.0));

    vec4 sum = vec4(0., 0., 0., 1.);

    vec3 pos = p + ray * jitter * stepLength;

    for (int i = 0; i < MAX_STEPS; i++)
    {
        if (sum.a < 0.1) {
            break;
        }
        float d = map(pos);

        if (d > 0.001)
        {
            vec3 lpos = pos + light * jitter * shadowStepLength;
            float shadow = 0.;

            for (int s = 0; s < SHADOW_STEPS; s++)
            {
                lpos += light * shadowStepLength;
                float lsample = map(lpos);
                shadow += lsample;
            }

            density = clamp((d / float(MAX_STEPS)) * 20.0, 0.0, 1.0);
            float s = exp((-shadow / float(SHADOW_STEPS)) * 3.);
            sum.rgb += vec3(s * density) * vec3(1.1, 0.9, .5) * sum.a;
            sum.a *= 1. - density;

            sum.rgb += exp(-map(pos + vec3(0, 0.25, 0.0)) * .2) * density * vec3(0.15, 0.45, 1.1) * sum.a;
        }
        pos += ray * stepLength;
    }

    return sum;
}


mat3 camera(vec3 ro, vec3 ta, float cr)
{
    vec3 cw = normalize(ta - ro);
    vec3 cp = vec3(sin(cr), cos(cr), 0.);
    vec3 cu = normalize(cross(cw, cp));
    vec3 cv = normalize(cross(cu, cw));
    return mat3(cu, cv, cw);
}

void main()
{
    vec2 p = (gl_FragCoord.xy * 2.0 - resolution.xy) / min(resolution.x, resolution.y);
    jitter = hash(p.x + p.y * 57.0 + iTime);
    mat3 viewRotation = mat3(view);

    vec3 ro = cameraPos;
    vec3 ta = vec3(0.0, 1., 0.0);
    vec3 ray = viewRotation * normalize(vec3(p, 1.0));
    vec4 col = cloudMarch(ro, ray);
    vec3 result = col.rgb + mix(vec3(0.3, 0.6, 1.0), vec3(0.05, 0.35, 1.0), p.y + 0.75) * (col.a);
    float sundot = clamp(dot(ray, normalize(vec3(1.0, 2.0, 1.0))), 0.0, 1.0);
    result += 0.4 * vec3(1.0, 0.7, 0.3) * pow(sundot, 4.0);

    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
