#version 460 core
#extension GL_ARB_bindless_texture : enable

in vec2 UV;
out float outColor;

layout(bindless_sampler) uniform sampler2D texturePosition;
layout(bindless_sampler) uniform sampler2D textureNormal;
layout(bindless_sampler) uniform sampler2D textureNoise;

layout(std140, binding = 1) uniform MeshData {
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 4) uniform UBOKernel {
    vec4 kernelData[64];
};

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

// tile noise texture over screen based on screen dimensions divided by noise size
uniform vec2 noiseScale; 


void main(void)
{
    vec3 position = vec3(view*vec4(texture(texturePosition, UV).rgb,1));
    vec3 randomVec = normalize(texture(textureNoise, UV * noiseScale).xyz);
    vec3 normal = normalize(texture(textureNormal, UV).rgb);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN*vec3(kernelData[i]); // from tangent to view-space
        samplePos = position + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = vec3(view*vec4(texture(texturePosition, offset.xy).xyz,1)).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    outColor=occlusion;
}