#include "../../include/SceneData/Animator.h"
#include "../../include/GlobalData/GlobalData.h"


Prisma::Animator::Animator(std::shared_ptr<Animation> animation)
{
	m_CurrentTime = 0.0;
	m_CurrentAnimation = animation;
}

void Prisma::Animator::UpdateAnimation(float dt)
{
	m_DeltaTime = dt;
	if (m_CurrentAnimation)
	{
		updateData = true;
		m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f),Prisma::AnimationHandler::getInstance().animations()[findUUID()]);
	}
}

void Prisma::Animator::PlayAnimation(std::shared_ptr<Animation> pAnimation)
{
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
}

void Prisma::Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, Prisma::AnimationHandler::SSBOAnimation& animation)
{
	const std::string& nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	auto Bone = m_CurrentAnimation->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(m_CurrentTime);
		nodeTransform = Bone->GetLocalTransform();
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
		CalculateBoneTransform(&node->children[i], globalTransformation,animation);
}

void Prisma::Animator::mesh(Node* mesh) {
	m_mesh = mesh;
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
