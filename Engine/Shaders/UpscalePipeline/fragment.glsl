#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D screenTexture;
uniform vec2 resolution;
uniform int factor;

void main()
{

	vec2 uv = floor(gl_FragCoord.xy / factor);

	uv = uv / (resolution / factor);


	vec4 sampledColor = texture(screenTexture, uv.xy);
    FragColor = sampledColor;
    gl_FragDepth = 0.999;

}