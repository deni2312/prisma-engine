#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"

uniform texture2D albedo;
uniform texture2D normal;
uniform texture2D position;

layout(location = 0) in vec2 TexCoords;

void main()
{
    vec4 diffuse = texture(sampler2D(albedo,textureClamp_sampler),TexCoords);
    vec4 normal = texture(sampler2D(normal,textureClamp_sampler),TexCoords);
    vec4 position = texture(sampler2D(position,textureClamp_sampler),TexCoords);

    float metallic=diffuse.a;
    float roughness=normal.a;

    vec3 color = pbrCalculation(position.rgb, normal.rgb, diffuse.rgb, vec4(1.0), roughness, metallic);
        
    FragColor = vec4(color,1);
}