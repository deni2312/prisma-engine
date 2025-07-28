#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"

uniform texture2D diffuseTexture;
uniform texture2D normalTexture;
uniform texture2D rmTexture;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(sampler2D(normalTexture,textureRepeat_sampler),outUv).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(outFragPos);
    vec3 Q2 = dFdy(outFragPos);
    vec2 st1 = dFdx(outUv);
    vec2 st2 = dFdy(outUv);

    vec3 N = normalize(outNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec4 diffuse = texture(sampler2D(diffuseTexture,textureRepeat_sampler),outUv);
    if(diffuse.a<0.1){
        discard;
    }
    // Sample the normal map
    //vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).rgb;
    
    // Remap from [0, 1] to [-1, 1]
    //tangentNormal = tangentNormal * 2.0 - 1.0;

    // Transform from tangent space to world space
    vec3 worldNormal = getNormalFromMap();

    vec4 rm = texture(sampler2D(rmTexture,textureRepeat_sampler),outUv);

    float metallic = rm.b;
    float roughness = rm.g;
    
    vec3 color = pbrCalculation(outFragPos, worldNormal, vec3(diffuse), vec4(1.0), roughness, metallic,1);
        
    FragColor = vec4(color,1);
}