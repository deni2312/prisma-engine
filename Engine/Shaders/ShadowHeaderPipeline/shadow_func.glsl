layout(std430, binding = 9) buffer CSMShadow
{
    float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

layout(std430, binding = 10) readonly buffer LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

float ShadowCalculation(vec3 fragPos, vec3 lightPos, int depthMapId)
{
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos.xyz - fragPos);
    float diskRadius = (1.0 + (viewDistance / omniData[depthMapId].far_plane.r)) / 25.0;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(omniData[depthMapId].depthMap, fragToLight + gridSamplingDisk[depthMapId] * diskRadius).r;
        closestDepth *= omniData[depthMapId].far_plane.r;   // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);


    return shadow;

}

float ShadowCalculationDirectional(vec3 fragPosWorldSpace, vec3 lightPos, vec3 N, unsigned int i)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < sizeCSM; ++i)
    {
        if (depthValue < cascadePlanes[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = int(sizeCSM);
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightPos)), 0.005);
    if (layer == int(sizeCSM))
    {
        bias *= 1 / (farPlaneCSM * directionalData[i].padding.y);
    }
    else
    {
        bias *= 1 / (cascadePlanes[layer] * directionalData[i].padding.y);
    }



    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(directionalData[i].depthMap, 0));
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(directionalData[i].depthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}