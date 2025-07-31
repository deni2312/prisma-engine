#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_ARB_shading_language_include : require

layout(location = 0) in vec2 outUv;
layout(location = 1) in vec3 outFragPos; // Fragment position in world space
layout(location = 2) in vec3 outNormal;
layout(location = 3) in flat int outDrawId;
layout(location = 4) in mat3 outTBN;

struct StatusData
{
    int status;
    int plainMaterial;
    int transparent;
    int isSpecular;
    vec3 GlassReflectionColorMask;
    float GlassAbsorption;
    vec4 GlassMaterialColor;
    vec2 GlassIndexOfRefraction;
    int GlassEnableDispersion;
    int DispersionSampleCount;
    float metalness;
    float roughness;
    vec2 padding;
};

readonly buffer statusData{
    StatusData statusData_data[];
};

uniform texture2D diffuseTexture[];
uniform texture2D normalTexture[];
uniform texture2D rmTexture[];

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

uniform sampler textureRepeat_sampler;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).xyz * 2.0 - 1.0;

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
    vec4 diffuse = texture(sampler2D(diffuseTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv);
    if(diffuse.a<0.1){
        discard;
    }


    // Sample the normal map
    //vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).rgb;
    
    // Remap from [0, 1] to [-1, 1]
    //tangentNormal = tangentNormal * 2.0 - 1.0;

    // Transform from tangent space to world space

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

    // store the fragment position vector in the first gbuffer texture
    gPosition.rgb = outFragPos;
    
    // Sample the normal map
    vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).rgb;
    //OpenGL to Vulkan
    tangentNormal.y=1-tangentNormal.y;
    // also store the per-fragment normals into the gbuffer
    gNormal.rgb = normalize(outTBN*normalize(tangentNormal * 2.0 - 1.0));

    gAlbedoSpec.rgb = diffuse.rgb;
    
    gAlbedoSpec.a= metallic;
    gNormal.a = roughness;
    gPosition.a = gl_FragCoord.z;

}