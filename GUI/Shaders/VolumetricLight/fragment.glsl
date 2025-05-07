#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2DArray csmShadow;
uniform sampler csmShadow_sampler;

void main()
{
    vec3 result = texture(sampler2DArray(csmShadow,csmShadow_sampler), vec3(TexCoords,0)).rgb;
    //float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    float brightness = max(max(result.r, result.g), result.b); // captures saturated colors
    if (brightness < 1.0) {
        result = vec3(0.0, 0.0, 0.0);
    }
    FragColor = vec4(result.r,1,1, 1.0);
}
