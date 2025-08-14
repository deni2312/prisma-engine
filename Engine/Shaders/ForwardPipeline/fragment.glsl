#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"
#include "../../../Engine/Shaders/ForwardPipeline/common.glsl"

uniform sampler textureAnisotropic_sampler;

layout(location = 4) in mat3 outTBN;

void main()
{
    vec4 diffuse = texture(sampler2D(diffuseTexture[nonuniformEXT(outDrawId)],textureAnisotropic_sampler),outUv);
    if(diffuse.a<0.1){
        discard;
    }
    // Sample the normal map
    vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureAnisotropic_sampler),outUv).rgb;

    //OpenGL to Vulkan
    //tangentNormal.y=1-tangentNormal.y;

    vec3 worldNormal = normalize(outTBN*normalize(tangentNormal * 2.0 - 1.0));

    vec4 rm = texture(sampler2D(rmTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);

    float metallic = rm.b;
    float roughness = rm.g;

    if(statusData_data[outDrawId].isSpecular==1){
        // Assume rm.rgb = specularColor (r,g,b), rm.a = glossiness
        vec3 specularColor = rm.rgb;
        float glossiness = rm.a;

        // Convert glossiness to roughness
        roughness = 1.0 - glossiness;

        // Approximate metallic from specular color
        // Metallic = max(specularColor.r, specularColor.g, specularColor.b)
        // This is a simplification — for accurate conversion you'd need IOR and energy conservation, but this is a decent heuristic
        metallic = max(max(specularColor.r, specularColor.g), specularColor.b);
    }

    metallic =  clamp(metallic+statusData_data[outDrawId].metalness,0,1);
    roughness = clamp(roughness+statusData_data[outDrawId].roughness,0,1);
    
    vec3 color = pbrCalculation(outFragPos, worldNormal, vec3(diffuse), vec4(1.0), roughness, metallic,1);
        
    FragColor = vec4(color,1);
}