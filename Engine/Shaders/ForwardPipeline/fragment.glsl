
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
flat in uint drawId;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

#include ../HelperHeaderPipeline/light_func.glsl
#include ../PbrHeaderPipeline/pbr_func.glsl
#include ../HelperHeaderPipeline/helper_func.glsl
#include ../ShadowHeaderPipeline/shadow_func.glsl
#include ../PbrHeaderPipeline/pbr_calculation.glsl

void main()
{
    #if defined(ANIMATE)
    currentMaterial = materialDataAnimation[drawId];
    #else
    currentMaterial = materialData[drawId];
    #endif

    vec4 diffuseTexture = texture(currentMaterial.diffuse, TexCoords);

    vec3 albedo = diffuseTexture.rgb;
    if (diffuseTexture.a < 0.1) {
        discard;
    }
    vec4 roughnessMetalnessTexture = texture(currentMaterial.roughness_metalness, TexCoords);
    float metallic = roughnessMetalnessTexture.b;
    float roughness = roughnessMetalnessTexture.g;
    float specularMap = texture(currentMaterial.specularMap, TexCoords).r;
    float ao = texture(currentMaterial.ambient_occlusion, TexCoords).r;

    vec4 aoSpecular=vec4(specularMap, ao, 0, 0);

    vec3 N = getNormalFromMap();

    vec3 pbrColor = pbrCalculation(FragPos, N, albedo, aoSpecular,roughness,metallic);

    FragColor = vec4(pbrColor, diffuseTexture.a);
}