#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

uniform Constants{
    vec4 resolution;
};

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 texelStep = 1.0 / resolution.xy;

    vec3 rgbM = texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords).rgb;
    float lumaM = luminance(rgbM);

    // Sample surrounding pixels
    float lumaN = luminance(texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords + vec2(0.0, texelStep.y)).rgb);
    float lumaS = luminance(texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords - vec2(0.0, texelStep.y)).rgb);
    float lumaE = luminance(texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords + vec2(texelStep.x, 0.0)).rgb);
    float lumaW = luminance(texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords - vec2(texelStep.x, 0.0)).rgb);

    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float lumaMax = max(lumaM, max(max(lumaN, lumaS), max(lumaE, lumaW)));

    // Compute edge direction
    vec2 dir;
    dir.x = -((lumaN + lumaS) - (2.0 * lumaM));
    dir.y =  ((lumaE + lumaW) - (2.0 * lumaM));

    float dirReduce = max((lumaN + lumaS + lumaE + lumaW) * 0.25 * 0.5, 1.0 / 128.0);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = clamp(dir * rcpDirMin, -8.0, 8.0) * texelStep;

    // Sample along the edge direction
    vec3 rgbA = 0.5 * (
        texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords + dir * (2.0 / 3.0 - 0.5)).rgb
    );
    vec3 rgbB = 0.25 * (
        texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords + dir * -0.5).rgb +
        texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords + dir * 0.5).rgb
    ) + 0.5 * rgbA;

    float lumaB = luminance(rgbB);
    FragColor = (lumaB < lumaMin || lumaB > lumaMax) ? vec4(rgbA, 1.0) : vec4(rgbB, 1.0);
}
