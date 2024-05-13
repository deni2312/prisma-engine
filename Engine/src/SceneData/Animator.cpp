#include "../../include/SceneData/Animator.h"
#include "../../include/GlobalData/GlobalData.h"
#include <glm/gtx/string_cast.hpp>


Prisma::Animator::Animator(std::shared_ptr<Animation> animation)
{
	m_CurrentTime = 0.0;
	m_CurrentAnimation = animation;
}

void Prisma::Animator::updateAnimation(float dt)
{
	if (m_CurrentAnimation)
	{
		updateLights = true;
		m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		calculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f),Prisma::AnimationHandler::getInstance().animations()[findUUID()]);
	}
}

void Prisma::Animator::playAnimation(std::shared_ptr<Animation> pAnimation)
{
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
}
glm::mat4 subtractMatrices(const glm::mat4& mat1, const glm::mat4& mat2) {
	glm::mat4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result[i][j] = mat1[i][j] - mat2[i][j];
		}
	}
	return result;
}

void Prisma::Animator::calculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, Prisma::AnimationHandler::SSBOAnimation& animation)
{
	const std::string& nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	auto Bone = m_CurrentAnimation->FindBone(nodeName);

	if (Bone.first)
	{
		Bone.first->Update(m_CurrentTime,Bone.second);
		nodeTransform = m_CurrentAnimation->getUpdate();
		std::cout << glm::to_string(subtractMatrices(nodeTransform,Bone.first->GetLocalTransform())) << std::endl;
	}
	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;
		animation.animations[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
		calculateBoneTransform(&node->children[i], globalTransformation,animation);
}

void Prisma::Animator::frame(float frame)
{
	if (m_CurrentAnimation)
	{
		updateLights = true;
		m_CurrentTime = frame;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		calculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), Prisma::AnimationHandler::getInstance().animations()[findUUID()]);
	}
}

void Prisma::Animator::mesh(Node* mesh) {
	m_mesh = mesh;
	updateAnimation(0);
}

std::shared_ptr<Prisma::Animation> Prisma::Animator::animation() {
	return m_CurrentAnimation;
}

float Prisma::Animator::currentTime() const {
	return m_CurrentTime;
}

int Prisma::Animator::findUUID()
{
	auto meshes = currentGlobalScene->animateMeshes;
	for (int i = 0; i < meshes.size(); i++) {
		if (meshes[i]->uuid() == m_mesh->uuid()) {
			return i;
		}
	}
	return 0;
}
