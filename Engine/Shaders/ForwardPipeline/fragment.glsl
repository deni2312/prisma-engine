#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"


void main()
{
    vec4 diffuse = texture(sampler2D(diffuseTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);
    vec3 normal = getNormalFromMap();

    vec4 rm = texture(sampler2D(rmTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);

    float metallic = rm.b;
    float roughness = rm.g;
    
    vec3 color = pbrCalculation(outFragPos, normal, vec3(diffuse), vec4(1.0), roughness, metallic);
        
    FragColor = vec4(color,1);
}