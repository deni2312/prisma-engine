#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform texture2D waterMaskTexture;
uniform texture2D positionTexture;
uniform sampler screenTexture_sampler; // Sampler for screenTexture

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos; // Camera's world position
};

// Constants for SSR
const int MAX_STEPS = 64;
const float STEP_SIZE = 0.1;
const float THICKNESS = 0.05;

// Reconstruct screen-space position to NDC
vec2 WorldToScreen(vec3 worldPos) {
    vec4 clipPos = projection * view * vec4(worldPos, 1.0);
    vec3 ndc = clipPos.xyz / clipPos.w;
    return ndc.xy * 0.5 + 0.5;
}

void main(void)
{
    vec3 albedo = texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords).rgb;
    float metallic = texture(sampler2D(waterMaskTexture, screenTexture_sampler), TexCoords).r;
    vec3 worldPos = texture(sampler2D(positionTexture, screenTexture_sampler), TexCoords).xyz;

    if (metallic < 0.01) {
        FragColor = vec4(albedo, 1.0);
        return;
    }

    // Fetch normal from waterMaskTexture and transform to world space
    vec3 normal = normalize((view * vec4(texture(sampler2D(waterMaskTexture, screenTexture_sampler), TexCoords).xyz, 0.0)).xyz);
    vec3 viewDir = normalize(viewPos.xyz - worldPos);

    // Reflect view direction around the normal
    vec3 reflDir = reflect(-viewDir, normal);

    vec3 reflPos = worldPos;
    vec3 lastPos = reflPos;

    vec3 result = vec3(0.0);
    bool hit = false;

    for (int i = 0; i < MAX_STEPS; ++i) {
        reflPos += reflDir * STEP_SIZE;

        vec2 screenUV = WorldToScreen(reflPos);
        if (screenUV.x < 0.0 || screenUV.x > 1.0 || screenUV.y < 0.0 || screenUV.y > 1.0)
            break;

        float sceneDepth = texture(sampler2D(positionTexture, screenTexture_sampler), screenUV).z;
        float depthDiff = reflPos.z - sceneDepth;

        if (abs(depthDiff) < THICKNESS) {
            result = texture(sampler2D(screenTexture, screenTexture_sampler), screenUV).rgb;
            hit = true;
            break;
        }

        lastPos = reflPos;
    }

    vec3 reflection = hit ? result : vec3(0.0);
    float reflectance = metallic; // Could be based on Fresnel/roughness

    FragColor = vec4(mix(albedo, reflection, reflectance), 1.0);
}
