#include "../../include/SceneData/Bone.h"
#include <memory>
#include "../../include/Helpers/Shader.h"
#include <glm/gtx/string_cast.hpp>

static std::shared_ptr<Prisma::Shader> m_shader = nullptr;
static unsigned int m_timelinePos;
static unsigned int m_indexAnimationPos;
static unsigned int m_positionPos;
static unsigned int m_rotationPos;
static unsigned int m_scalePos;


Prisma::Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
	:
	m_Name(name),
	m_ID(ID),
	m_LocalTransform(1.0f)
{
	m_NumPositions = channel->mNumPositionKeys;

	if (!m_shader) {
		m_shader = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/compute.glsl");
		m_shader->use();
		m_timelinePos = m_shader->getUniformPosition("timeline");
		m_indexAnimationPos = m_shader->getUniformPosition("indexAnimation");
		m_positionPos = m_shader->getUniformPosition("numPositions");
		m_rotationPos = m_shader->getUniformPosition("numRotations");
		m_scalePos = m_shader->getUniformPosition("numScales");
	}

	for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data;
		data.position = getVec(aiPosition);
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
		data.timeStamp.r = timeStamp;
		m_Rotations.push_back(data);
	}

	m_NumScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		KeyScale data;
		data.scale = getVec(scale);
		data.timeStamp = timeStamp;
		m_Scales.push_back(data);
	}
}

void Prisma::Bone::Update(float animationTime, int index)
{
	m_shader->use();
	m_shader->dispatchCompute(glm::ivec3(1, 1, 1));
	m_shader->setFloat(m_timelinePos, animationTime);
	m_shader->setInt(m_positionPos, m_NumPositions);
	m_shader->setInt(m_rotationPos, m_NumRotations);
	m_shader->setInt(m_scalePos, m_NumScalings);
	m_shader->setInt(m_indexAnimationPos, index);

	m_shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
	glm::mat4 translation = InterpolatePosition(animationTime);
	glm::mat4 rotation = InterpolateRotation(animationTime);
	glm::mat4 scale = InterpolateScaling(animationTime);
	m_LocalTransform = translation * rotation * scale;
}

glm::mat4 Prisma::Bone::GetLocalTransform() { return m_LocalTransform; }

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
		if (animationTime < m_Rotations[index + 1].timeStamp.r)
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

std::vector<Prisma::KeyPosition>& Prisma::Bone::positions() {
	return m_Positions; 
}

std::vector<Prisma::KeyRotation>& Prisma::Bone::rotations() {
	return m_Rotations; 
}

std::vector<Prisma::KeyScale>& Prisma::Bone::scales() {
	return m_Scales; 
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
		return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

	int p0Index = GetPositionIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
		m_Positions[p1Index].timeStamp, animationTime);
	glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position
		, scaleFactor);


	return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Prisma::Bone::InterpolateRotation(float animationTime) const
{
	if (1 == m_NumRotations)
	{
		auto rotation = glm::normalize(m_Rotations[0].orientation);
		return glm::toMat4(rotation);
	}
	int p0Index = GetRotationIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp.r,
		m_Rotations[p1Index].timeStamp.r, animationTime);
	glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation
		, scaleFactor);
	finalRotation = glm::normalize(finalRotation);
	return glm::toMat4(finalRotation);

}

glm::mat4 Prisma::Bone::InterpolateScaling(float animationTime) const
{
	if (1 == m_NumScalings)
		return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

	int p0Index = GetScaleIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
		m_Scales[p1Index].timeStamp, animationTime);
	glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
		, scaleFactor);
	return glm::scale(glm::mat4(1.0f), finalScale);
}
