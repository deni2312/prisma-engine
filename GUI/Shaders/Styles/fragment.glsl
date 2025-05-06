#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform sampler screenTexture_sampler;

uniform Constants{
    ivec4 currentStyle;
};

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
        color=cartoon();
    }
    FragColor = vec4(color, 1.0);
}
