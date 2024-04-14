#version 460 core
#extension GL_ARB_bindless_texture : enable

in vec2 UV;
out vec4 outColor;

layout(bindless_sampler)uniform sampler2D textureFrame;
layout(bindless_sampler)uniform sampler2D textureNorm;
layout(bindless_sampler)uniform sampler2D textureMetallic;

uniform float rayStep = 0.2f;
uniform int iterationCount = 100;
uniform float distanceBias = 0.05f;
uniform bool enableSSR = true;
uniform int sampleCount = 4;
uniform bool isSamplingEnabled = false;
uniform bool isExponentialStepEnabled = false;
uniform bool isAdaptiveStepEnabled = true;
uniform bool isBinarySearchEnabled = true;
uniform bool debugDraw = false;
uniform float samplingCoefficient;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

float random (vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

vec3 generatePositionFromDepth(vec2 texturePos, float depth) {
    vec4 ndc = vec4((texturePos - 0.5) * 2, depth, 1.f);
    vec4 inversed = inverse(projection) * ndc;// going back from projected
    inversed /= inversed.w;
    return inversed.xyz;
}

vec2 generateProjectedPosition(vec3 pos){
    vec4 samplePosition = projection * vec4(pos, 1.f);
    samplePosition.xy = (samplePosition.xy / samplePosition.w) * 0.5 + 0.5;
    return samplePosition.xy;
}

vec3 SSR(vec3 position, vec3 reflection) {
    vec3 step = rayStep * reflection;
    vec3 marchingPosition = position + step;
    float delta;
    float depthFromScreen;
    vec2 screenPosition;

    int i = 0;
    for (; i < iterationCount; i++) {
        screenPosition = generateProjectedPosition(marchingPosition);
        depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(textureFrame, screenPosition).w).z);
        delta = abs(marchingPosition.z) - depthFromScreen;
        if (abs(delta) < distanceBias) {
            vec3 color = vec3(1);
            if(debugDraw)
            color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
            return texture(textureFrame, screenPosition).xyz * color;
        }
        if (isBinarySearchEnabled && delta > 0) {
            break;
        }
        if (isAdaptiveStepEnabled){
            float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
            //this is sort of adapting step, should prevent lining reflection by doing sort of iterative converging
            //some implementation doing it by binary search, but I found this idea more cheaty and way easier to implement
            step = step * (1.0 - rayStep * max(directionSign, 0.0));
            marchingPosition += step * (-directionSign);
        }
        else {
            marchingPosition += step;
        }
        if (isExponentialStepEnabled){
            step *= 1.05;
        }
    }
    if(isBinarySearchEnabled){
        for(; i < iterationCount; i++){

            step *= 0.5;
            marchingPosition = marchingPosition - step * sign(delta);

            screenPosition = generateProjectedPosition(marchingPosition);
            depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(textureFrame, screenPosition).w).z);
            delta = abs(marchingPosition.z) - depthFromScreen;

            if (abs(delta) < distanceBias) {
                vec3 color = vec3(1);
                if(debugDraw)
                color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
                return texture(textureFrame, screenPosition).xyz * color;
            }
        }
    }

    return vec3(0.0);
}

void main(){
    vec3 position = generatePositionFromDepth(UV, texture(textureFrame, UV).w);
    vec4 normal = view * vec4(texture(textureNorm, UV).xyz, 0.0);
    float metallic = texture(textureMetallic, UV).w;
    if (!enableSSR || metallic < 0.01) {
        outColor = texture(textureFrame, UV);
    } else {
        vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));
        if (isSamplingEnabled) {
            vec3 firstBasis = normalize(cross(vec3(0.f, 0.f, 1.f), reflectionDirection));
            vec3 secondBasis = normalize(cross(reflectionDirection, firstBasis));
            vec4 resultingColor = vec4(0.f);
            for (int i = 0; i < sampleCount; i++) {
                vec2 coeffs = vec2(random(UV + vec2(0, i)) + random(UV + vec2(i, 0))) * samplingCoefficient;
                vec3 reflectionDirectionRandomized = reflectionDirection + firstBasis * coeffs.x + secondBasis * coeffs.y;
                vec3 tempColor = SSR(position, normalize(reflectionDirectionRandomized));
                if (tempColor != vec3(0.f)) {
                    resultingColor += vec4(tempColor, 1.f);
                }
            }
            if (resultingColor.w == 0){
                outColor = texture(textureFrame, UV);
            } else {
                resultingColor /= resultingColor.w;
                outColor = vec4(resultingColor.xyz, 1.f);
            }
        }
        else {
            outColor = vec4(SSR(position, normalize(reflectionDirection)), 1.f);
            if (outColor.xyz == vec3(0.f)){
                outColor = texture(textureFrame, UV);
            }
        }
    }
}