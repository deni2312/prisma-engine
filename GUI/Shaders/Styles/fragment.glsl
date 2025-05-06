#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

uniform Constants{
    ivec4 currentStyle;
};

vec3 vignette(){
    float strength = 0.5;
    vec2 texCoordsNormalized = TexCoords * 2.0 - 1.0;
    float radius = length(texCoordsNormalized);
    vec3 color = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;

    // Vignette calculation
    float vignette = smoothstep(1.0, 1.0 - strength, radius);

    return color * vignette;
}

vec3 seppia(){
    // Sample the input texture
    vec4 texColor = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords);

    // Sepia tone conversion
    float r = texColor.r * 0.393 + texColor.g * 0.769 + texColor.b * 0.189;
    float g = texColor.r * 0.349 + texColor.g * 0.686 + texColor.b * 0.168;
    float b = texColor.r * 0.272 + texColor.g * 0.534 + texColor.b * 0.131;

    // Make sure values are clamped between 0 and 1
    r = clamp(r, 0.0, 1.0);
    g = clamp(g, 0.0, 1.0);
    b = clamp(b, 0.0, 1.0);

    // Output the final color
    return vec3(r, g, b);
}

vec3 cartoon(){
        vec3 color= vec3(0.0);
        // Get the size of a single texel
        vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
        color = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;
        color.r=currentStyle.r;
        // Sobel edge detection
        vec3 sobel = vec3(0.0);
        sobel += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + texelSize * vec2(-1, -1)).rgb * 1.0;
        sobel += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + texelSize * vec2(-1, 0)).rgb * 2.0;
        sobel += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + texelSize * vec2(-1, 1)).rgb * 1.0;
        sobel += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + texelSize * vec2(1, -1)).rgb * -1.0;
        sobel += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + texelSize * vec2(1, 0)).rgb * -2.0;
        sobel += texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords + texelSize * vec2(1, 1)).rgb * -1.0;

        float sobelLength = length(sobel);
        vec3 edge = vec3(step(0.4, sobelLength)); // Adjust the threshold for edge detection

        // Reduce number of colors
        color = floor(color * 5.0) / 5.0;

        // Combine color and edge
        color = mix(color, vec3(0.0), edge);
        return color;
}

void main()
{
    vec3 color= vec3(0.0);
    if(currentStyle.r==1){
        color=seppia();
    }
    if(currentStyle.r==2){
        color=cartoon();
    }
    if(currentStyle.r==3){
        color=vignette();
    }
    FragColor = vec4(color, 1.0);
}
