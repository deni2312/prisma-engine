#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"


void main()
{
    vec4 diffuse = texture(sampler2D(diffuseTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);
    // Sample the normal map
    //vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).rgb;
    
    // Remap from [0, 1] to [-1, 1]
    //tangentNormal = tangentNormal * 2.0 - 1.0;

    // Transform from tangent space to world space
    vec3 worldNormal = getNormalFromMap();

    vec4 rm = texture(sampler2D(rmTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);

    float metallic = rm.b;
    float roughness = rm.g;
    
    vec3 color = pbrCalculation(outFragPos, worldNormal, vec3(diffuse), vec4(1.0), roughness, metallic);
        
    FragColor = vec4(color,1);
}