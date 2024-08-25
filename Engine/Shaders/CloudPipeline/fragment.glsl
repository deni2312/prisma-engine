#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 cameraPos;
uniform float iTime;
uniform vec2 resolution;
uniform vec3 lightDir;
layout(bindless_sampler) uniform sampler2D uNoise;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

#define MAX_STEPS 100

float sdSphere(vec3 p, float radius) {
    return length(p) - radius;
}

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    vec2 uv = (p.xy + vec2(37.0, 239.0) * p.z) + f.xy;
    vec2 tex = texture(uNoise, (uv + 0.5) / 256.0).yx;

    return mix(tex.x, tex.y, f.z) * 2.0 - 1.0;
}

float fbm(vec3 p) {
    vec3 q = p + iTime * 0.5 * vec3(1.0, -0.2, -1.0);
    float g = noise(q);

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

float scene(vec3 p) {
    float distance = sdSphere(p, 1.0);

    float f = fbm(p);

    return -distance + f;
}

const float MARCH_SIZE = 0.08;

vec4 raymarch(vec3 rayOrigin, vec3 rayDirection) {
    float depth = 0.0;
    vec3 p = rayOrigin + depth * rayDirection;

    vec4 res = vec4(0.0);

    for (int i = 0; i < MAX_STEPS; i++) {
        float density = scene(p);

        // We only draw the density if it's greater than 0
        if (density > 0.0) {
            vec4 color = vec4(mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), density), density);
            color.rgb *= color.a;
            res += color * (1.0 - res.a);
        }

        depth += MARCH_SIZE;
        p = rayOrigin + depth * rayDirection;
    }

    return res;
}


void main()
{
    vec2 ndc = (gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0;
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

    vec3 color = vec3(0.0);
    vec4 res = raymarch(ro, rd);
    color = res.rgb;

    FragColor = vec4(color, 1.0);
}
