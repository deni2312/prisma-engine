#version 460 core
#extension GL_ARB_bindless_texture : enable

in vec2 UV;
out vec4 outColor;

layout(bindless_sampler) uniform sampler2D textureAlbedo;
layout(bindless_sampler) uniform sampler2D textureNorm;
layout(bindless_sampler) uniform sampler2D texturePosition;

layout(std140, binding = 1) uniform MeshData {
    mat4 view;
    mat4 projection;
};

uniform vec2 screenSize; // Screen size

const int numSamples = 128;
const float inverseResolution = 0.001; // Hardcoded ratio.x value

void main(void)
{
    vec4 color = vec4(0.0);
    vec4 surfaceData = texture(textureNorm, UV);
    float depth = surfaceData.w;

    vec3 eyeSpaceNormal = normalize(surfaceData.xyz);
    vec3 eyeSpacePosition = normalize(texture(texturePosition, UV).xyz);

    vec4 viewDir = inverse(projection) * vec4(0.0, 0.0, 1.0, 1.0);
    viewDir /= viewDir.w;

    vec4 reflectionVector = normalize(projection * vec4(reflect(viewDir.xyz, eyeSpaceNormal), 1.0));
    float refLength = length(reflectionVector.xy);
    reflectionVector = normalize(reflectionVector) / refLength * inverseResolution;
    reflectionVector.z *= depth;

    vec3 currentPosition = vec3(reflectionVector.x, reflectionVector.y, depth + reflectionVector.z);

    int currentSamples = 0;
    while (currentSamples < numSamples) {
        float sampledDepth = texture(textureNorm, UV + currentPosition.xy).w;
        float difference = currentPosition.z - sampledDepth;
        if (difference > 0.0025) {
            color = texture(textureAlbedo, UV + currentPosition.xy);
            break;
        }
        currentPosition += reflectionVector.xyz;
        currentSamples++;
    }

    outColor = vec4(color.rgb, 1.0);
}
