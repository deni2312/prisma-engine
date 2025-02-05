#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

float volumetricLightStrength = 10.0;

float layer = 0.0;

layout(bindless_sampler) uniform sampler2D screenTexture;

layout(std430, binding = 9) buffer CSMShadow
{
    float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

struct DirectionalData
{
	vec4 direction;
	vec4 diffuse;
	vec4 specular;
	sampler2DArray depthMap;
	vec2 padding;
};

layout(std140, binding = 3) uniform FragmentData
{
    vec4 viewPos;
    samplerCube irradianceMap;
    samplerCube prefilterMap;
    sampler2D brdfLUT;
    vec2 paddingFragment;
    sampler2D textureLut;
    sampler2D textureM;
};

layout(std430, binding = 2) buffer Directional
{
	ivec4 lenDir;
	DirectionalData directionalData[];
};

layout(std140, binding = 1) uniform MeshData
{
	mat4 view;
	mat4 projection;
};

void main()
{
    // Get the color from the screen texture
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // Calculate the view direction from the camera to the current fragment
    vec3 viewDir = normalize(vec3(TexCoords, 1.0) - viewPos.xyz);

    // Calculate the distance from the camera to the current fragment
    float distance = length(TexCoords - viewPos.xy);

    // Calculate the volumetric density using a simple function (e.g., exponential decay)
    float density = exp(-distance * 0.01); // Adjust the coefficient to control density falloff

    // Apply Beer's law to attenuate the light based on density
    float lightAttenuation = exp(-density * volumetricLightStrength);

    // Apply the attenuated light to the color
    color *= lightAttenuation;

    // Output the final color

    FragColor = vec4(color, 1.0);
}
