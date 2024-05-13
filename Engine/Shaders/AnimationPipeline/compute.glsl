#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

const int MAX_BONES = 128;

struct AnimationData {
    vec4 positions[MAX_BONES];
    vec4 rotations[MAX_BONES];
    vec4 timestamp[MAX_BONES];
    vec4 scales[MAX_BONES];

};

uniform float timeline;
uniform int indexAnimation;
uniform int numPositions;
uniform int numRotations;
uniform int numScales;

layout(std430, binding = 11) buffer AnimationMatrices
{
    AnimationData animationData[];
};

layout(std430, binding = 12) buffer TransformData
{
    mat4 transform;
};

float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

int GetPositionIndex(float animationTime)
{
    for (int index = 0; index < numPositions - 1; ++index)
    {
        if (animationTime < animationData[indexAnimation].positions[index + 1].w)
            return index;
    }
}

int GetRotationIndex(float animationTime)
{
    for (int index = 0; index < numRotations - 1; ++index)
    {
        if (animationTime < animationData[indexAnimation].timestamp[index + 1].x)
            return index;
    }
}

int GetScaleIndex(float animationTime)
{
    for (int index = 0; index < numScales - 1; ++index)
    {
        if (animationTime < animationData[indexAnimation].scales[index + 1].w)
            return index;
    }
}

mat4 InterpolatePosition(float animationTime) {
    if (1 == numPositions) {
        mat4 result = mat4(1.0);
        result[3] = vec4(animationData[indexAnimation].positions[0].xyz, 1.0);
        return result;
    }

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(animationData[indexAnimation].positions[p0Index].w,
        animationData[indexAnimation].positions[p1Index].w, animationTime);
    vec3 finalPosition = mix(animationData[indexAnimation].positions[p0Index].xyz, animationData[indexAnimation].positions[p1Index].xyz, scaleFactor);
    mat4 result = mat4(1.0);
    result[3] = vec4(animationData[indexAnimation].positions[p0Index].xyz, 1.0);

    return result;
}

mat4 toMat4(vec4 q)
{
    mat4 Result = mat4(1.0);
    float qxx = q.x * q.x;
    float qyy = q.y * q.y;
    float qzz = q.z * q.z;
    float qxz = q.x * q.z;
    float qxy = q.x * q.y;
    float qyz = q.y * q.z;
    float qwx = q.w * q.x;
    float qwy = q.w * q.y;
    float qwz = q.w * q.z;

    Result[0][0] = 1.0 - 2.0 * (qyy + qzz);
    Result[0][1] = 2.0 * (qxy + qwz);
    Result[0][2] = 2.0 * (qxz - qwy);

    Result[1][0] = 2.0 * (qxy - qwz);
    Result[1][1] = 1.0 - 2.0 * (qxx + qzz);
    Result[1][2] = 2.0 * (qyz + qwx);

    Result[2][0] = 2.0 * (qxz + qwy);
    Result[2][1] = 2.0 * (qyz - qwx);
    Result[2][2] = 1.0 - 2.0 * (qxx + qyy);
    return Result;
}

vec4 slerp(vec4 x, vec4 y, float a)
{
    vec4 z = y;

    float cosTheta = dot(x, y);

    // If cosTheta < 0, the interpolation will take the long way around the sphere.
    // To fix this, one vec4 must be negated.
    if (cosTheta < 0.0)
    {
        z = -y;
        cosTheta = -cosTheta;
    }

    // Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
    if (cosTheta > 1.0 - 0.0001)
    {
        // Linear interpolation
        return mix(x, z, a);
    }
    else
    {
        // Essential Mathematics, page 467
        float angle = acos(cosTheta);
        return (sin((1.0 - a) * angle) * x + sin(a * angle) * z) / sin(angle);
    }
}

mat4 InterpolateRotation(float animationTime)
{
    if (1 == numRotations)
    {
        vec4 rotation = normalize(animationData[indexAnimation].rotations[0]);
        return toMat4(rotation);
    }
    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(animationData[indexAnimation].timestamp[p0Index].r,
        animationData[indexAnimation].timestamp[p1Index].r, animationTime);
    vec4 finalRotation = slerp(animationData[indexAnimation].rotations[p0Index], animationData[indexAnimation].rotations[p1Index]
        , scaleFactor);
    
    finalRotation = normalize(finalRotation);
    return toMat4(finalRotation);
}

mat4 scale(vec3 scaleFactor)
{
    return mat4(
        vec4(scaleFactor.x, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor.y, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor.z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

mat4 InterpolateScaling(float animationTime)
{
    if (1 == numScales)
        return scale(animationData[indexAnimation].scales[0].xyz);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(animationData[indexAnimation].scales[p0Index].w,
        animationData[indexAnimation].scales[p1Index].w, animationTime);
    vec3 finalScale = mix(animationData[indexAnimation].scales[p0Index].xyz, animationData[indexAnimation].scales[p1Index].xyz
        , scaleFactor);
    return scale(finalScale);
}

void main()
{
    transform = mat4(1.0);
    transform = InterpolatePosition(timeline) * InterpolateRotation(timeline) * InterpolateScaling(timeline);
}