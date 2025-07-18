#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"

layout(location = 0) in vec2 outUv;
layout(location = 1) in vec3 outFragPos;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outLocalPos;

layout(location = 1) out vec4 Reflection;

uniform textureCube skybox;

// Simple hash noise function (optional)
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f*f*(3.0 - 2.0*f);
    return mix(a, b, u.x) + (c - a)*u.y*(1.0 - u.x) + (d - b)*u.x*u.y;
}

void main()
{
    vec3 shallowColor = vec3(0.1, 0.4, 0.6);
    vec3 deepColor = vec3(0.0, 0.05, 0.2);

    // Direction vectors
    vec3 I = normalize(outFragPos - viewPos.xyz);
    vec3 N = normalize(outNormal);

    // Fresnel approximation (Schlick)
    float fresnel = pow(1.0 - max(dot(-I, N), 0.0), 5.0);
    fresnel = mix(0.02, 1.0, fresnel);

    // Reflect environment
    vec3 reflectedColor = texture(samplerCube(skybox, textureClamp_sampler), reflect(I, N)).rgb;

    // Simulated depth by height
    float heightFactor = clamp((outLocalPos.y + 1.0) * 0.5, 0.0, 1.0);
    vec3 baseColor = mix(deepColor, shallowColor, heightFactor);

    // -------- FOAM CALCULATION --------
    float foam = 0.0;

    // Height-based foam at peaks (tweak threshold as needed)
    float foamHeight = clamp((outLocalPos.y - 0.2) * 5.0, 0.0, 1.0);

    // Optional animated noise foam breakup (move with time or UV)
    float foamNoise = noise(outUv);

    // Combine foam factors
    foam = foamHeight * foamNoise;

    // Blend white foam into final color
    vec3 foamColor = mix(baseColor, vec3(1.0), foam);

    // -------- FINAL COLOR MIX --------
    vec3 finalColor = mix(foamColor, reflectedColor, fresnel);

    FragColor = vec4(finalColor, 1.0);
    Reflection=vec4(N,1);
}
