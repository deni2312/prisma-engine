#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

uniform Constants{
    ivec4 currentStyle;
    vec2 resolution;
    vec2 time;
};

#define MAX_STEPS 128
#define MARCH_SIZE 0.08

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    vec2 uv = (p.xy + vec2(37.0, 239.0) * p.z) + f.xy;
    vec2 tex = texture(sampler2D(screenTexture,screenTexture_sampler), (uv + 0.5) / 512.0).yx;

    return mix(tex.x, tex.y, f.z) * 2.0 - 1.0;
}

float fbm(vec3 p) {
    vec3 q = p + time.r * 0.5 * vec3(1.0, -0.2, -1.0);
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

float sdSphere(vec3 p, float radius) {
    return length(p) - radius;
}

float scene(vec3 p) {
    float distance = sdSphere(p, 1.0);
    float f = fbm(p);
    return -distance + f;
}

vec4 raymarch(vec3 ro, vec3 rd) {
    float depth = 0.0;
    vec3 p;
    vec4 accumColor = vec4(0.0);

    for (int i = 0; i < MAX_STEPS; i++) {
        p = ro + depth * rd;
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

    return accumColor;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    // Camera setup
    vec3 ro = vec3(0.0, 0.0, 3.0);
    vec3 rd = normalize(vec3(uv, -1.0));

    vec4 result = raymarch(ro, rd);
    FragColor = result;
}
