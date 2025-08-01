
uniform texture2D diffuseTexture[];
uniform texture2D normalTexture[];
uniform texture2D rmTexture[];

uniform sampler textureRepeat_sampler;

readonly buffer statusData{
    StatusData statusData_data[];
};


layout(location = 0) in vec2 outUv;
layout(location = 1) in vec3 outFragPos; // Fragment position in world space
layout(location = 2) in vec3 outNormal;
layout(location = 3) in flat int outDrawId;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(sampler2D(normalTexture[nonuniformEXT(outDrawId)],textureRepeat_sampler),outUv).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(outFragPos);
    vec3 Q2 = dFdy(outFragPos);
    vec2 st1 = dFdx(outUv);
    vec2 st2 = dFdy(outUv);

    vec3 N = normalize(outNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}