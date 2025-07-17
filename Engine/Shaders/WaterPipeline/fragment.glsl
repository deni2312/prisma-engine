#extension GL_ARB_shading_language_include : require

#include "../../../Engine/Shaders/PbrHeaderPipeline/pbr_calculation.glsl"

void main()
{    
    vec3 color = pbrCalculation(vec3(1), vec3(1), vec3(1), vec4(1.0), 1, 1);
        
    FragColor = vec4(vec3(1),color.r);
}