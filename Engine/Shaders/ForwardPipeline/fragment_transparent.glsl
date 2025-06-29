#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"
#include "../../../Engine/Shaders/ForwardPipeline/common.glsl"


layout(location = 1) out float reveal;
void main()
{
    vec4 diffuse = texture(sampler2D(diffuseTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);
    if(diffuse.a<0.1){
        discard;
    }
    // Sample the normal map
    //vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).rgb;
    
    // Remap from [0, 1] to [-1, 1]
    //tangentNormal = tangentNormal * 2.0 - 1.0;

    // Transform from tangent space to world space
    vec3 worldNormal = getNormalFromMap();

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

    metallic =  metallic+statusData_data[outDrawId].metalness;
    roughness = roughness+statusData_data[outDrawId].roughness;
    
    vec3 color = pbrCalculation(outFragPos, worldNormal, vec3(diffuse), vec4(1.0), roughness, metallic);
    float weight = clamp(pow(min(1.0, diffuse.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    reveal = diffuse.a;
    FragColor = vec4(color.rgb * diffuse.a, diffuse.a) * weight;
}