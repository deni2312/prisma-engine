

in vec3 FragPos;
in vec2 TexCoords;
in vec3 viewPos;
in vec3 Normal;
flat in uint drawId;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec4 gAmbient;

struct MaterialData {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D roughness_metalness;
    sampler2D specularMap;
    sampler2D ambient_occlusion;
    int transparent;
    float padding;
    vec4 materialColor;
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

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(currentMaterial.normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

struct StatusData{
    uint status;
    int plainColor;
    vec2 padding;
};

layout(std430, binding = 25) buffer StatusCopy {
    StatusData statusCopy[];
};


void main()
{
#if defined(ANIMATE)
    currentMaterial = materialDataAnimation[drawId];
#else
    currentMaterial = materialData[drawId];
#endif


    vec4 albedoTexture = texture(currentMaterial.diffuse, TexCoords)+currentMaterial.materialColor;

    if (albedoTexture.a < 0.1) {
        discard;
    }
    if(statusCopy[drawId].plainColor>1){
        gAmbient.b= float(statusCopy[drawId].plainColor);
        gAlbedoSpec.rgb = vec3(currentMaterial.materialColor);
    }else{
        // and the diffuse per-fragment color
        gAlbedoSpec.rgb = albedoTexture.rgb;

        // store the fragment position vector in the first gbuffer texture
        gPosition.rgb = FragPos;
        // also store the per-fragment normals into the gbuffer
        gNormal.rgb = getNormalFromMap();

        vec4 roughnessMetalnessTexture = texture(currentMaterial.roughness_metalness, TexCoords);

        gAlbedoSpec.a= roughnessMetalnessTexture.b;
        gNormal.a = roughnessMetalnessTexture.g;
        gPosition.a=gl_FragCoord.z;

        gAmbient.r = texture(currentMaterial.specularMap, TexCoords).r;
        gAmbient.g = texture(currentMaterial.ambient_occlusion, TexCoords).r;
        gAmbient.b = 0;
    }
}