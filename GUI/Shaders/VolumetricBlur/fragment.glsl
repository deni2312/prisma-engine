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

uniform BlurData{
    float exposure;
    float decay;
    float density;
    float weight;
    ivec4 numSamples;
};

void main()
{
    if(dirSize>0){
        vec3 lightDir = -normalize(dirData_data[0].direction.xyz); // World-space direction

        vec3 eyePos = viewPos.rgb; // The camera is at the origin in view space
        vec2 coord = TexCoords;
        float illuminationDecay = 1.0;

        vec3 color = vec3(0.0);
        vec3 sunWorldPos = eyePos + lightDir * 100.0; // Arbitrary distant point along the light direction

        // Project into clip space
        vec4 sunClipPos = projection * view * vec4(sunWorldPos, 1.0);

        // Perspective divide to get NDC
        vec3 sunNDC = sunClipPos.xyz / sunClipPos.w;

        // Convert to screen space [0,1]
        vec2 lightScreenPos = sunNDC.xy * 0.5 + 0.5;
        vec2 deltaTexCoord = TexCoords - lightScreenPos;
        deltaTexCoord *= 1.0 / float(numSamples.r) * density;
        for(int i = 0; i < numSamples.r; ++i)
        {
            coord -= deltaTexCoord;
            vec3 occlusionTexture = texture(sampler2D(screenTexture,screenTexture_sampler), coord).rgb;
            occlusionTexture *= illuminationDecay * weight;

            color += occlusionTexture;

            illuminationDecay *= decay;
        }

        FragColor = vec4(color * exposure, 1.0);
    }else{
        FragColor=vec4(0.0);
    }
}
