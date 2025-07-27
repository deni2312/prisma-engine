#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D noiseTexture;
uniform texture2D normalTexture;
uniform texture2D positionTexture;

uniform sampler screenTexture_sampler;
uniform sampler positionTexture_sampler;

uniform Constants{
    vec4 samples[64];
};

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
const int kernelSize = 64;
const float radius = 0.5;
const float bias = 0.025;


void main()
{
    vec2 noiseScale = vec2(WIDTH/4.0, HEIGHT/4.0); 

    // get input for SSAO algorithm
    vec3 fragPos = vec3(view*vec4(texture(sampler2D(positionTexture,positionTexture_sampler), TexCoords).xyz,1));
    vec3 normal = vec3(view*vec4(normalize(texture(sampler2D(normalTexture,positionTexture_sampler), TexCoords).rgb),1));
    vec3 randomVec = normalize(texture(sampler2D(noiseTexture,positionTexture_sampler), TexCoords * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i].xyz; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.y = -offset.y; // Flip Y to match Vulkan’s NDC
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(sampler2D(positionTexture,positionTexture_sampler), offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = vec4(vec3(occlusion),1);
}
