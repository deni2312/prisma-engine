#include "../../include/SceneData/Bone.h"

Prisma::Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
	:
	m_LocalTransform(1.0f),
	m_Name(name),
	m_ID(ID)
{
	m_NumPositions = channel->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data;
		data.position = getVec3(aiPosition);
		data.timeStamp = timeStamp;
		m_Positions.push_back(data);
	}

	m_NumRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		KeyRotation data;
		data.orientation = getQuat(aiOrientation);
		data.timeStamp = timeStamp;
		m_Rotations.push_back(data);
	}

	m_NumScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		KeyScale data;
		data.scale = getVec3(scale);
		data.timeStamp = timeStamp;
		m_Scales.push_back(data);
	}
}

void Prisma::Bone::Update(float animationTime)
{
	glm::mat4 translation = InterpolatePosition(animationTime);
	glm::mat4 rotation = InterpolateRotation(animationTime);
	glm::mat4 scale = InterpolateScaling(animationTime);
	m_LocalTransform = translation * rotation * scale;
}

const glm::mat4& Prisma::Bone::GetLocalTransform() const { return m_LocalTransform; }

std::string Prisma::Bone::GetBoneName() const { return m_Name; }

int Prisma::Bone::GetBoneID() { return m_ID; }

int Prisma::Bone::GetPositionIndex(float animationTime) const
{
	for (int index = 0; index < m_NumPositions - 1; ++index)
	{
		if (animationTime < m_Positions[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

int Prisma::Bone::GetRotationIndex(float animationTime) const
{
	for (int index = 0; index < m_NumRotations - 1; ++index)
	{
		if (animationTime < m_Rotations[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

int Prisma::Bone::GetScaleIndex(float animationTime) const
{
	for (int index = 0; index < m_NumScalings - 1; ++index)
	{
		if (animationTime < m_Scales[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

float Prisma::Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const
{
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}

glm::mat4 Prisma::Bone::InterpolatePosition(float animationTime) const
{
	if (1 == m_NumPositions)
		return translate(glm::mat4(1.0f), m_Positions[0].position);

	int p0Index = GetPositionIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
	                                   m_Positions[p1Index].timeStamp, animationTime);
	glm::vec3 finalPosition = mix(m_Positions[p0Index].position, m_Positions[p1Index].position
	                              , scaleFactor);
	return translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Prisma::Bone::InterpolateRotation(float animationTime) const
{
	if (1 == m_NumRotations)
	{
		auto rotation = normalize(m_Rotations[0].orientation);
		return toMat4(rotation);
	}
	int p0Index = GetRotationIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
	                                   m_Rotations[p1Index].timeStamp, animationTime);
	glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation
	                                     , scaleFactor);
	finalRotation = normalize(finalRotation);
	return toMat4(finalRotation);
}

glm::mat4 Prisma::Bone::InterpolateScaling(float animationTime) const
{
	if (1 == m_NumScalings)
		return scale(glm::mat4(1.0f), m_Scales[0].scale);

	int p0Index = GetScaleIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
	                                   m_Scales[p1Index].timeStamp, animationTime);
	glm::vec3 finalScale = mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
	                           , scaleFactor);
	return scale(glm::mat4(1.0f), finalScale);
}
