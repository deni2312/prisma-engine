#version 460 core
#extension GL_ARB_bindless_texture : enable

in vec2 UV;
out vec4 outColor;

layout(bindless_sampler) uniform sampler2D textureAlbedo;
layout(bindless_sampler) uniform sampler2D textureNorm;
layout(bindless_sampler) uniform sampler2D texturePosition;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform vec2 screenSize; // Screen size

// Function to perform ray marching
vec3 rayMarch(vec3 startPos, vec3 rayDir) {
    const int maxSteps = 50; // Max steps for ray marching
    const float stepSize = 0.1; // Step size
    vec3 pos = startPos;

    for (int i = 0; i < maxSteps; i++) {
        pos += rayDir * stepSize;

        // Project the position back to screen space
        vec4 projPos = projection * vec4(pos, 1.0);
        projPos /= projPos.w;
        vec2 screenUV = (projPos.xy * 0.5) + 0.5;

        // Check if the position is within screen bounds
        if (screenUV.x < 0.0 || screenUV.x > 1.0 || screenUV.y < 0.0 || screenUV.y > 1.0) {
            break;
        }

        // Sample the depth buffer at this screen position
        float sampledDepth = texture(texturePosition, screenUV).z;

        // If the sampled depth is greater than the current position's depth, we have hit a surface
        if (sampledDepth < pos.z) {
            return pos;
        }
    }

    // If no hit is found, return a large value to indicate no intersection
    return vec3(1e8);
}

void main() {
    // Sample textures
    vec4 albedo = texture(textureAlbedo, UV);
    vec3 normal = texture(textureNorm, UV).xyz;
    vec3 position = texture(texturePosition, UV).xyz;

    // Extract metalness from albedo's alpha channel
    float metalness = albedo.a;

    // View direction
    vec3 viewDir = normalize(position - vec3(view[3]));

    // Reflect view direction around the normal
    vec3 reflectionDir = reflect(viewDir, normal);

    // Perform ray marching to find the reflected position
    vec3 reflectedPos = rayMarch(position, reflectionDir);

    // Check if reflection hit something
    if (reflectedPos.z < 1e8) {
        // Project the reflected position to screen space
        vec4 reflectedProjPos = projection * vec4(reflectedPos, 1.0);
        reflectedProjPos /= reflectedProjPos.w;
        vec2 reflectedUV = (reflectedProjPos.xy * 0.5) + 0.5;

        // Sample the albedo at the reflected position
        vec4 reflectedColor = texture(textureAlbedo, reflectedUV);

        // Mix the reflected color with the original albedo based on metalness
        outColor = mix(vec4(albedo.xyz,1.0), reflectedColor, metalness);
    }
    else {
        // If no reflection, just use the original albedo
        outColor = vec4(albedo.xyz,1.0);
    }
}
