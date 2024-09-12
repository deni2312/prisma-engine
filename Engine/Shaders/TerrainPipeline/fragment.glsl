#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

in float Height;

in vec3 FragPos;
in vec3 Normal;

in vec2 textureCoord;

layout(bindless_sampler) uniform sampler2D grass;
layout(bindless_sampler) uniform sampler2D stone;
layout(bindless_sampler) uniform sampler2D snow;

uniform float mult;
uniform float shift;

layout(std430, binding = 10) readonly buffer LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(std430, binding = 5) restrict buffer clusterSSBO
{
    Cluster clusters[];
};

struct DirectionalData
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
    sampler2DArray depthMap;
    vec2 padding;
};

layout(std140, binding = 2) uniform ClusterData
{
    uvec4 gridSize;
    uvec4 screenDimensions;
    float zNear;
    float zFar;
    float padding[2];
};

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 3) uniform FragmentData
{
    vec4 viewPos;
    samplerCube irradianceMap;
    samplerCube prefilterMap;
    sampler2D brdfLUT;
    vec2 paddingFragment;
};

struct OmniData {
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 far_plane;
    vec4 attenuation;
    samplerCube depthMap;
    float padding;
    float radius;
};

layout(std430, binding = 2) buffer Directional
{
    vec4 lenDir;
    DirectionalData directionalData[];
};

layout(std430, binding = 3) buffer Omni
{
    vec4 lenOmni;
    OmniData omniData[];
};

layout(std430, binding = 9) buffer CSMShadow
{
    float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};


void main()
{
    // Normalize height range (-16 to 48) to (0 to 1)
    float normalizedHeight = (Height - shift) / mult;

    // Define height thresholds for different layers
    float grassHeight = 0.3;  // Below 30% height, it's mostly grass
    float rockHeight = 0.7;   // Between 30% and 70%, it's rock
    float snowHeight = 1.0;   // Above 70%, snow

    // Blend factors between layers
    float grassFactor = smoothstep(0.0, grassHeight, normalizedHeight);
    float rockFactor = smoothstep(grassHeight, rockHeight, normalizedHeight);
    float snowFactor = smoothstep(rockHeight, snowHeight, normalizedHeight);

    // Sample textures based on TexCoords
    vec4 grassColor = texture(grass, textureCoord);
    vec4 rockColor = texture(stone, textureCoord);
    vec4 snowColor = texture(snow, textureCoord);

    // Blend textures based on height
    vec4 blendedColor = mix(grassColor, rockColor, rockFactor);  // Grass to Rock blend
    blendedColor = mix(blendedColor, snowColor, snowFactor);     // Rock to Snow blend

    // Additional effects based on normal direction (for snow accumulation on flat surfaces)
    float slopeFactor = dot(Normal, vec3(0.0, 1.0, 0.0));  // How flat the surface is
    slopeFactor = clamp(slopeFactor, 0.0, 1.0);            // Ensure slopeFactor is between 0 and 1

    // Accumulate more snow on flat surfaces
    blendedColor = mix(blendedColor, snowColor, slopeFactor * snowFactor);

    // Output final color
    FragColor = blendedColor;
}