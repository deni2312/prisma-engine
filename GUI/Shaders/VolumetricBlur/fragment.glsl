#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform texture2D depthTexture;
uniform texture2D noiseTexture;
uniform sampler screenTexture_sampler;

struct DirectionalData
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    float hasShadow;
    float bias;
    vec2 padding;
};

readonly buffer dirData{
    DirectionalData dirData_data[];
};

uniform LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

// === Volumetric Fog Uniforms ===
uniform FogSettings {
    vec4 fogColor;

    vec4 maxDistance;
    vec4 stepSize;
    vec4 densityMultiplier;
    vec4 noiseOffset;

    vec4 densityThreshold;
    vec4 noiseTiling;

    vec4 lightContribution;
    vec4 lightScattering;
};

const float PI = 3.14159265359;

// === Henyey-Greenstein phase function ===
float henyeyGreenstein(float angle, float g) {
    return (1.0 - g * g) / (4.0 * PI * pow(1.0 + g * g - 2.0 * g * angle, 1.5));
}

// === Get volumetric density from noise ===
float getDensity(vec3 worldPos) {
    // Project world position onto XZ plane (you can also use XY or YZ)
    vec2 uv = worldPos.xz * 0.01 * noiseTiling.r;
    vec4 noise = texture(sampler2D(noiseTexture, screenTexture_sampler), uv);
    float d = dot(noise.rgb, noise.rgb);
    return max((d - densityThreshold.r), 0.0) * densityMultiplier.r;
}

// === Reconstruct world position from screen UV and depth ===
vec3 reconstructWorldPos(vec2 uv, float depth) {
    vec2 newUvs=vec2(uv.x,1-uv.y);

    vec4 ndc = vec4(newUvs * 2.0 - 1.0, depth, 1.0);
    vec4 clip = inverse(projection) * ndc;
    vec4 view = inverse(view) * (clip / clip.w);
    return view.xyz;
}

void main() {
    vec3 col = texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords).rgb;
    float depthVal = texture(sampler2D(depthTexture, screenTexture_sampler), TexCoords).r;

    vec3 worldPos = reconstructWorldPos(TexCoords, depthVal);
    vec3 camPos = viewPos.xyz;
    vec3 viewDir = worldPos - camPos;
    float viewLength = length(viewDir);
    vec3 rayDir = normalize(viewDir);

    float distLimit = min(viewLength, maxDistance.r);
    
    // Simple hash noise based offset
    float hash = fract(sin(dot(TexCoords, vec2(12.9898, 78.233))) * 43758.5453);
    float distTravelled = hash * noiseOffset.r;

    float transmittance = 1.0;
    vec3 fogAccum = fogColor.rgb;

    while (distTravelled < distLimit) {
        vec3 rayPos = camPos + rayDir * distTravelled;
        float density = getDensity(rayPos);

        if (density > 0.0 && dirSize > 0) {
            vec3 lightDir = normalize(-dirData_data[0].direction.xyz); // World-space
            vec3 lightCol = dirData_data[0].diffuse.rgb;
            float phase = henyeyGreenstein(dot(rayDir, lightDir), lightScattering.r);
            float shadow = 1.0; // Shadowing would be integrated here
            fogAccum += lightCol * lightContribution.rgb * phase * density * shadow * stepSize.r;
            transmittance *= exp(-density * stepSize.r);
        }

        distTravelled += stepSize.r;
    }

    vec3 finalColor = mix(col, fogAccum, 1.0 - clamp(transmittance, 0.0, 1.0));
    FragColor = vec4(worldPos, finalColor.r);
}