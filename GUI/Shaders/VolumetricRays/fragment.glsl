#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
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

readonly buffer dirData {
    DirectionalData dirData_data[];
};

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

uniform RaysData{
    vec4 exposure;
    vec4 decay;
    vec4 density;
    vec4 weight;
    ivec4 samples;
};

uniform LightSizes
{
    int omniSize;
    int dirSize;
    int areaSize;
    int padding;
};

vec2 getLightScreenPos() {
    // Get directional light direction (world space)
    vec3 lightDir = normalize(-dirData_data[0].direction.xyz);

    // Choose a point far along light direction from camera position
    float dist = 100.0; // Far away point along light dir
    vec3 lightPosWorld = viewPos.xyz + lightDir * dist;

    // Project light position to clip space
    vec4 lightClip = projection * view * vec4(lightPosWorld, 1.0);

    // Perspective divide
    vec3 ndc = lightClip.xyz / lightClip.w;

    // Convert from NDC [-1..1] to UV [0..1]
    vec2 lightUV = ndc.xy * 0.5 + 0.5;

    return lightUV;
}

void main() {
    if(dirSize>0){
        vec2 lightScreenPos = getLightScreenPos();

        vec2 deltaTex = lightScreenPos - TexCoords;
        vec2 stepTex = deltaTex * density.r / float(samples.r);

        vec2 uv = TexCoords;
        float illuminationDecay = 1.0;
        vec3 color = vec3(0.0);

        for(int i = 0; i < samples.r; i++) {
            uv += stepTex;

            // Sample scene texture, check if uv inside [0,1]
            if(uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
                break;

            vec3 sampleColor = texture(sampler2D(screenTexture, screenTexture_sampler), uv).rgb;

            sampleColor *= illuminationDecay * weight.r;
            color += sampleColor;

            illuminationDecay *= decay.r;
        }

        color *= exposure.r;

        vec3 original = texture(sampler2D(screenTexture, screenTexture_sampler), TexCoords).rgb;

        // Additive blend god rays
        vec3 finalColor = original + color;

        FragColor = vec4(finalColor, 1.0);
    }else{
        FragColor = vec4(0.0);
    }
}