#include "../../include/SceneData/Animator.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/CacheScene.h"
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
		Prisma::CacheScene::getInstance().updateLights(true);
		m_CurrentTime += m_CurrentAnimation->ticksPerSecond() * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->duration());

		// Perform the blend if in blending mode
		if (m_IsBlending && m_PreviousAnimation)
		{
			// Increase blend factor over time
			m_BlendFactor += dt / m_BlendDuration;

			if (m_BlendFactor >= 1.0f)
			{
				// Stop blending when blend factor reaches 1.0
				m_BlendFactor = 1.0f;
				m_IsBlending = false;
				m_PreviousAnimation.reset(); // No longer needed
				m_PreviousAnimation = nullptr;
			}

			// Calculate and blend bone transforms
			blendAnimations(m_BlendFactor);
		}
		else
		{
			// No blending, use current animation directly
			calculateBoneTransform(&m_CurrentAnimation->rootNode(), glm::mat4(1.0f),
				Prisma::AnimationHandler::getInstance().animations()[findUUID()]);
		}
	}
}

void Prisma::Animator::playAnimation(std::shared_ptr<Animation> pAnimation,float blendDuration)
{
	m_CurrentTime = 0.0f;
	m_PreviousAnimation = m_CurrentAnimation;
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
	m_BlendFactor = 0.0f;
	calculateCurrentTransform(&m_CurrentAnimation->rootNode(), glm::mat4(1.0f));
	auto transform = Prisma::AnimationHandler::getInstance().animations()[findUUID()];
	for (int i = 0; i < MAX_BONES; i++) {
		m_currentTransform[i] = transform.animations[i];
	}
	m_BlendDuration = blendDuration;
	m_IsBlending = true;
}

void Prisma::Animator::calculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, Prisma::AnimationHandler::SSBOAnimation& animation)
{
	const std::string& nodeName = node->name;
	glm::mat4 nodeTransform;

	auto Bone = m_CurrentAnimation->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(m_CurrentTime);
		nodeTransform = Bone->GetLocalTransform();
	}
	else {
		nodeTransform = node->transformation;
	}
	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	const auto& boneInfoMap = m_CurrentAnimation->boneIdMap();
	auto it = boneInfoMap->find(nodeName);
	if (it != boneInfoMap->end()) {
		const auto& boneInfo = (*it).second;
		animation.animations[boneInfo.id] = globalTransformation * boneInfo.offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
		calculateBoneTransform(&node->children[i], globalTransformation,animation);
}

void Prisma::Animator::frame(float frame)
{
	if (m_CurrentAnimation)
	{
		Prisma::CacheScene::getInstance().updateLights(true);
		m_CurrentTime = frame;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->duration());
		calculateBoneTransform(&m_CurrentAnimation->rootNode(), glm::mat4(1.0f), Prisma::AnimationHandler::getInstance().animations()[findUUID()]);
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

void Prisma::Animator::blendAnimations(float blendFactor)
{
	auto& currentBones = Prisma::AnimationHandler::getInstance().animations()[findUUID()];

	for (int i = 0; i < MAX_BONES; i++)
	{
		currentBones.animations[i]= decomposeAndInterpolateMat4(m_currentTransform[i], m_previousTransform[i], blendFactor);
	}
}

void Prisma::Animator::calculateCurrentTransform(const AssimpNodeData* node, const glm::mat4& parentTransform)
{
	const std::string& nodeName = node->name;
	glm::mat4 nodeTransform;

	auto Bone = m_CurrentAnimation->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(0);
		nodeTransform = Bone->GetLocalTransform();
	}
	else {
		nodeTransform = node->transformation;
	}
	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	const auto& boneInfoMap = m_CurrentAnimation->boneIdMap();
	auto it = boneInfoMap->find(nodeName);
	if (it != boneInfoMap->end()) {
		const auto& boneInfo = (*it).second;
		m_previousTransform[boneInfo.id] = globalTransformation * boneInfo.offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
		calculateCurrentTransform(&node->children[i], globalTransformation);
}

glm::mat4 Prisma::Animator::decomposeAndInterpolateMat4(const glm::mat4& matA, const glm::mat4& matB, float t) {
	glm::vec3 scaleA, scaleB, translationA, translationB, skewA, skewB;
	glm::vec4 perspectiveA, perspectiveB;
	glm::quat rotationA, rotationB;

	// Decompose both matrices
	glm::decompose(matA, scaleA, rotationA, translationA, skewA, perspectiveA);
	glm::decompose(matB, scaleB, rotationB, translationB, skewB, perspectiveB);

	// Interpolate each component
	glm::vec3 scaleInterp = glm::mix(scaleA, scaleB, t);
	glm::vec3 translationInterp = glm::mix(translationA, translationB, t);
	glm::quat rotationInterp = glm::slerp(rotationA, rotationB, t);

	// Recompose the matrix
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleInterp);
	glm::mat4 rotationMatrix = glm::mat4_cast(rotationInterp);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translationInterp);

	return translationMatrix * rotationMatrix * scaleMatrix;
}
