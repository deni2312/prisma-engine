//#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.hlsl"
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_texture_array : require

layout(location = 0) out vec4 FragColor;


layout(location = 0) in vec2 outUv;
layout(location = 1) in vec3 outFragPos; // Fragment position in world space
layout(location = 2) in vec3 outNormal;
layout(location = 3) in flat int outDrawId;


uniform sampler2D diffuseTexture[];

SamplerState diffuseTexture_Sampler;

//sampler2D normalTexture[];

//sampler2D rmTexture[];

void main()
{
    vec4 diffuse = texture(diffuseTexture, vec3(outUv, float(outDrawId)));
    //vec3 normal = GetNormalFromMap(normalTexture[PSIn.drawId], texture_sampler, PSIn.UV, PSIn.FragPos, PSIn.NormalPS);

    //vec4 rm = rmTexture[PSIn.drawId].Sample(texture_sampler, PSIn.UV);

    //float metallic = rm.b;
    //float roughness = rm.g;
    
    //vec3 color = pbrCalculation(PSIn.FragPos, normal, (vec3)diffuse,PSIn.Pos, vec4(1.0), roughness, metallic);
        
    FragColor = diffuse;
}