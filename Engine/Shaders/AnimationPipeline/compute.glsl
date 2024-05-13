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
    for (int index = 0; index < numPositions - 1; ++index)
    {
        if (animationTime < animationData[indexAnimation].timestamp[index + 1].x)
            return index;
    }
}

int GetScaleIndex(float animationTime)
{
    for (int index = 0; index < numPositions - 1; ++index)
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
    result[3] = vec4(animationData[indexAnimation].positions[p0Index].xyz, scaleFactor);

    return result;
}

void main()
{
    transform = InterpolatePosition(timeline);
}