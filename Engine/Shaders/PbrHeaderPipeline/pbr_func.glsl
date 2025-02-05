const float PI = 3.14159265359;

struct MaterialData {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D roughness_metalness;
    sampler2D specularMap;
    sampler2D ambient_occlusion;
    int transparent;
    float padding;
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


MaterialData currentMaterial;

#if defined(ANIMATE)
layout(std430, binding = 7) readonly buffer MaterialAnimation
{
    MaterialData materialDataAnimation[];
};
#else
layout(std430, binding = 0) readonly buffer Material
{
    MaterialData materialData[];
};
#endif

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}